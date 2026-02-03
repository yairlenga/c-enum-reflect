/*
 * gcc_enum_reflect.cc  (GCC 13+)
 *
 * - Records enums passed to enum_reflect(x) (x must be enum-typed expression)
 * - At end of translation unit, emits:
 *     __enum_lblstr_<E>  (char blob: "A\0B\0...\0" + 8 NUL)
 *     __enum_lbloff_<E>  (uint16 offsets into lblstr)
 *     __enum_vals_<E>    (int values)
 *     __enum_desc_<E>    (const struct enum_desc, using the real type from headers)
 *
 * Build:
 *   g++ -shared -fPIC -O2 -fno-lto -fno-rtti -fno-exceptions \
 *     -o gcc_enum_reflect.so gcc_enum_reflect.cc \
 *     -I"$(gcc -print-file-name=plugin)/include"
 */

#include "gcc-plugin.h"
#include "plugin-version.h"

#include "context.h"
#include "tree.h"
#include "stringpool.h"

#include "gimple.h"
#include "gimple-iterator.h"
#include "tree-pass.h"
#include "basic-block.h"
#include "diagnostic.h"

#include "cgraph.h"
#include "varasm.h"
#include "stor-layout.h"

#include "hash-set.h"
#include "hash-map.h"

#include <cstring>
#include <string>
#include <vector>
#include <map>

#define streq(a, b) (strcmp((a), (b)) == 0)
    // Uncomment for debug output
#define dprintf(...)
// #define dprintf(...) // fprintf(stderr, __VA_ARGS__)

int plugin_is_GPL_compatible;

/* ------------------------------------------------------------ */
/* Globals we discover/collect */

static tree g_enum_desc_record = NULL_TREE;   /* RECORD_TYPE for struct enum_desc */
static bool field_lookup_done ;
static struct enum_desc_fields {
    tree f_strs;
    tree f_value_count;
    tree f_lbl_off;
    tree f_values;
} g_enum_desc_fields;

static hash_set<tree> g_seen_enums;           /* ENUMERAL_TYPE nodes to emit */

static const char *kReflectFnName = "enum_desc_gen";  // magic function to expand

/* ------------------------------------------------------------ */
/* Small helpers */

static const char *type_name_cstr(tree t)
{
    if (!t) return "<null>";

    tree tn = TYPE_NAME(t);
    if (!tn) return "<anonymous>";

    if (TREE_CODE(tn) == TYPE_DECL)
    {
        tree id = DECL_NAME(tn);
        return id ? IDENTIFIER_POINTER(id) : "<anonymous>";
    }

    if (TREE_CODE(tn) == IDENTIFIER_NODE)
        return IDENTIFIER_POINTER(tn);

    return "<anonymous>";
}

static const char *fndecl_name_cstr(tree fndecl)
{
    if (!fndecl || TREE_CODE(fndecl) != FUNCTION_DECL) return nullptr;
    tree id = DECL_NAME(fndecl);
    return id ? IDENTIFIER_POINTER(id) : nullptr;
}

static tree make_u16_type()
{
#ifdef uint16_type_node
    return uint16_type_node;
#else
    static tree t = nullptr;
    if (!t) t = build_nonstandard_integer_type(16, /*unsigned=*/1);
    return t;
#endif
}

static tree ptr_to_first_elem(tree array_expr, tree desired_ptr_type)
{
    // array_expr has ARRAY_TYPE, desired_ptr_type is pointer to element type
    tree idx0 = build_int_cst(size_type_node, 0);

    // Build array_expr[0]
    tree elem = build4(ARRAY_REF,
                       TREE_TYPE(desired_ptr_type),   // element type
                       array_expr,
                       idx0,
                       NULL_TREE,
                       NULL_TREE);

    // Take &array_expr[0]
    tree addr = build_fold_addr_expr(elem);

    // Cast if needed
    return fold_convert(desired_ptr_type, addr);
}


/* ------------------------------------------------------------ */
/* Extract labels and int values from ENUMERAL_TYPE reliably */

struct enum_item_kv {
    std::string label;
    HOST_WIDE_INT value;
};

static bool extract_enum_items(tree enum_type, std::vector<enum_item_kv> &out)
{
    out.clear();

    if (!enum_type || TREE_CODE(enum_type) != ENUMERAL_TYPE)
        return false;

    for (tree n = TYPE_VALUES(enum_type); n; n = TREE_CHAIN(n))
    {
        // Common representation: TREE_LIST with (purpose=name, value=INTEGER_CST or CONST_DECL)
        if (TREE_CODE(n) == TREE_LIST)
        {
            tree id  = TREE_PURPOSE(n);
            tree val = TREE_VALUE(n);

            const char *name =
                (id && TREE_CODE(id) == IDENTIFIER_NODE) ? IDENTIFIER_POINTER(id) : nullptr;
            if (!name) continue;

            if (val && TREE_CODE(val) == CONST_DECL)
                val = DECL_INITIAL(val);

            if (!val || TREE_CODE(val) != INTEGER_CST)
                continue;

            HOST_WIDE_INT v = TREE_INT_CST_LOW(val);
            out.push_back({name, v});
            continue;
        }

        // Sometimes: CONST_DECL nodes directly
        if (TREE_CODE(n) == CONST_DECL)
        {
            tree id = DECL_NAME(n);
            const char *name = id ? IDENTIFIER_POINTER(id) : nullptr;
            if (!name) continue;

            tree init = DECL_INITIAL(n);
            if (!init || TREE_CODE(init) != INTEGER_CST)
                continue;

            HOST_WIDE_INT v = TREE_INT_CST_LOW(init);
            out.push_back({name, v});
            continue;
        }
    }

    return !out.empty();
}

/* Build lbl_str and lbl_off with 8 NUL padding */
static bool build_lbl_blob(const std::vector<enum_item_kv> &items,
                           std::string &blob,
                           std::vector<uint16_t> &offs,
                           const char *ename_for_errors)
{
    blob.clear();
    offs.clear();
    offs.reserve(items.size());

    blob.append(ename_for_errors);
    blob.push_back('\0');

    for (auto &it : items)
    {
        offs.push_back((uint16_t)blob.size());
        blob.append(it.label);
        blob.push_back('\0');
    }

    blob.append(8, '\0');  // required padding
    if (blob.size() >= 65536)
    {
        error("enum %s: %<lbl_str%> too large for uint16 offsets", ename_for_errors);
        return false;
    }
    return true;
}

/* ------------------------------------------------------------ */
/* Emit const arrays */

static tree emit_const_char_blob(const char *sym, const std::string &blob)
{
    tree cch = build_qualified_type(char_type_node, TYPE_QUAL_CONST);
    tree arr_t = build_array_type_nelts(cch, (unsigned)blob.size());

    tree var = build_decl(BUILTINS_LOCATION, VAR_DECL, get_identifier(sym), arr_t);
    TREE_STATIC(var) = 1;
    TREE_READONLY(var) = 1;
    DECL_ARTIFICIAL(var) = 1;
    TREE_USED(var) = 1;

    vec<constructor_elt, va_gc> *elts = NULL;
    for (unsigned i = 0; i < (unsigned)blob.size(); i++)
    {
        tree idx = build_int_cst(integer_type_node, (int)i);
        unsigned char b = (unsigned char)blob[i];
        tree bv = build_int_cst(char_type_node, (int)b);
        CONSTRUCTOR_APPEND_ELT(elts, idx, bv);
    }

    DECL_INITIAL(var) = build_constructor(arr_t, elts);
    varpool_node::finalize_decl(var);
    return var;
}

static tree emit_const_u16_array(const char *sym, const std::vector<uint16_t> &a)
{
    tree u16 = make_u16_type();
    tree elem_t = build_qualified_type(u16, TYPE_QUAL_CONST);
    tree arr_t = build_array_type_nelts(elem_t, (unsigned)a.size());

    tree var = build_decl(BUILTINS_LOCATION, VAR_DECL, get_identifier(sym), arr_t);
    TREE_STATIC(var) = 1;
    TREE_READONLY(var) = 1;
    DECL_ARTIFICIAL(var) = 1;
    TREE_USED(var) = 1;

    vec<constructor_elt, va_gc> *elts = NULL;
    for (unsigned i = 0; i < a.size(); i++)
    {
        tree idx = build_int_cst(integer_type_node, (int)i);
        tree vv  = build_int_cst(u16, (int)a[i]);
        CONSTRUCTOR_APPEND_ELT(elts, idx, vv);
    }

    DECL_INITIAL(var) = build_constructor(arr_t, elts);
    varpool_node::finalize_decl(var);
    return var;
}

static tree emit_const_int_array(const char *sym, const std::vector<enum_item_kv> &items)
{
    tree elem_t = build_qualified_type(integer_type_node, TYPE_QUAL_CONST);
    tree arr_t  = build_array_type_nelts(elem_t, (unsigned)items.size());

    tree var = build_decl(BUILTINS_LOCATION, VAR_DECL, get_identifier(sym), arr_t);
    TREE_STATIC(var) = 1;
    TREE_READONLY(var) = 1;
    DECL_ARTIFICIAL(var) = 1;
    TREE_USED(var) = 1;

    vec<constructor_elt, va_gc> *elts = NULL;
    for (unsigned i = 0; i < items.size(); i++)
    {
        tree idx = build_int_cst(integer_type_node, (int)i);
        tree vv  = build_int_cst(integer_type_node, items[i].value);
        CONSTRUCTOR_APPEND_ELT(elts, idx, vv);
    }

    DECL_INITIAL(var) = build_constructor(arr_t, elts);
    varpool_node::finalize_decl(var);
    return var;
}

/* ------------------------------------------------------------ */
/* Emit enum_desc (real type) */

static tree field_by_name(tree record_type, const char *fname)
{
    for (tree f = TYPE_FIELDS(record_type); f; f = DECL_CHAIN(f))
    {
        if (TREE_CODE(f) != FIELD_DECL) continue;
        tree id = DECL_NAME(f);
        if (!id) continue;
        const char *n = IDENTIFIER_POINTER(id);
        if (n && streq(n, fname))
            return f;
    }
    return NULL_TREE;
}

static void lookup_fields(tree record_type)
{
    if (field_lookup_done)
        return;

    g_enum_desc_fields = {
        .f_strs = field_by_name(record_type, "strs"),
        .f_value_count = field_by_name(record_type, "value_count"),
        .f_lbl_off = field_by_name(record_type, "lbl_off"),
        .f_values = field_by_name(record_type, "values")
    };

    if (!g_enum_desc_fields.f_strs ||
        !g_enum_desc_fields.f_value_count ||
        !g_enum_desc_fields.f_lbl_off ||
        !g_enum_desc_fields.f_values)
    {
        error("struct %<enum_desc%> missing expected fields");
    }

    field_lookup_done = true;
}

static void emit_enum_desc_for(tree enum_type)
{
    if (!g_enum_desc_record)
    {
        error("struct %<enum_desc%> not visible (include the header defining struct %<enum_desc%>)");
        return;
    }
    lookup_fields(g_enum_desc_record);

    const char *ename = type_name_cstr(enum_type);
    dprintf("%s: emitting enum_desc for %s\n", __func__, ename);

    std::vector<enum_item_kv> items;
    if (!extract_enum_items(enum_type, items))
    {
        warning(0, "enum %s: no items extracted", ename);
        return;
    }

    std::string blob;
    std::vector<uint16_t> offs;
    if (!build_lbl_blob(items, blob, offs, ename))
        return;

    char sym_lbl[256], sym_off[256], sym_val[256], sym_desc[256];
    snprintf(sym_lbl,  sizeof(sym_lbl),  "__enum_lblstr_%s", ename);
    snprintf(sym_off,  sizeof(sym_off),  "__enum_lbloff_%s", ename);
    snprintf(sym_val,  sizeof(sym_val),  "__enum_vals_%s",   ename);
    snprintf(sym_desc, sizeof(sym_desc), "__enum_desc__%s",   ename);

    tree lbl_var = emit_const_char_blob(sym_lbl, blob);
    tree off_var = emit_const_u16_array(sym_off, offs);
    tree val_var = emit_const_int_array(sym_val, items);

    // Create desc var with the *real* type
    tree desc_var = build_decl(BUILTINS_LOCATION, VAR_DECL,
                               get_identifier(sym_desc), g_enum_desc_record);
    TREE_STATIC(desc_var) = 1;
    TREE_READONLY(desc_var) = 1;
    DECL_ARTIFICIAL(desc_var) = 1;
    TREE_USED(desc_var) = 1;

    vec<constructor_elt, va_gc> *elts = NULL;
    hash_map<tree, tree> fv ;
//    struct enum_desc_fields &f = g_enum_desc_fields ;
    struct enum_desc_fields &f = g_enum_desc_fields ;
    fv.put(f.f_value_count, fold_convert(TREE_TYPE(f.f_value_count), build_int_cst(integer_type_node, (int)items.size())));
    fv.put(f.f_values, ptr_to_first_elem(val_var, TREE_TYPE(f.f_values)));
    fv.put(f.f_lbl_off, ptr_to_first_elem(off_var, TREE_TYPE(f.f_lbl_off)));
    fv.put(f.f_strs, ptr_to_first_elem(lbl_var, TREE_TYPE(f.f_strs)));
    for (tree f = TYPE_FIELDS(g_enum_desc_record); f; f = DECL_CHAIN(f))
    {
        tree *s = fv.get(f);
        if ( s ) CONSTRUCTOR_APPEND_ELT(elts, f, *s) ;
    }

    // Anything not explicitly mentioned is zero-initialized by the constructor.
    DECL_INITIAL(desc_var) = build_constructor(g_enum_desc_record, elts);
    varpool_node::finalize_decl(desc_var);
}

/* ------------------------------------------------------------ */
/* Detect calls and remember enum types */

static void process_enum_reflect_call(gimple *stmt)
{
    tree fndecl = gimple_call_fndecl(stmt);
    const char *fname = fndecl_name_cstr(fndecl);
    if (!fname || streq(fname, kReflectFnName))
        return;

    dprintf("%s: found call to %s\n", __func__, kReflectFnName);

    if (gimple_call_num_args(stmt) != 1)
        return;

    tree arg = gimple_call_arg(stmt, 0);
    if (!arg) return;

    tree type = TREE_TYPE(arg);
    if (!type) return;

    type = TYPE_MAIN_VARIANT(type);

    if (TREE_CODE(type) != ENUMERAL_TYPE)
    {
        error_at(gimple_location(stmt),
                 "argument to %<enum_reflect%> must be an enum-typed expression");
        return;
    }

    // Remember for later emission
    g_seen_enums.add(type);
}

/* ------------------------------------------------------------ */
/* GIMPLE pass */

static const pass_data enum_refl_pass_data = {
    GIMPLE_PASS,
    "enum_refl",
    OPTGROUP_NONE,
    TV_NONE,
    0, 0, 0,
    0, 0
};

namespace {

class enum_refl_pass final : public gimple_opt_pass {
public:
    explicit enum_refl_pass(gcc::context *ctx)
        : gimple_opt_pass(enum_refl_pass_data, ctx)
    {}

    unsigned int execute(function *) override
    {
        basic_block bb;
        FOR_EACH_BB_FN(bb, cfun)
        {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb);
                 !gsi_end_p(gsi);
                 gsi_next(&gsi))
            {
                gimple *stmt = gsi_stmt(gsi);
                if (is_gimple_call(stmt))
                    process_enum_reflect_call(stmt);
            }
        }
        return 0;
    }
};

} // namespace

static const char *record_tag_name(tree rec)
{
    if (!rec) return nullptr;

    // TYPE_NAME(RECORD_TYPE) can be TYPE_DECL or IDENTIFIER_NODE
    tree tn = TYPE_NAME(rec);
    if (!tn) return nullptr;

    if (TREE_CODE(tn) == TYPE_DECL)
        tn = DECL_NAME(tn);

    if (!tn || TREE_CODE(tn) != IDENTIFIER_NODE)
        return nullptr;

    return IDENTIFIER_POINTER(tn);
}

static void on_finish_type(void *event_data, void *)
{
    if ( g_enum_desc_record ) return; // already found

    tree t = (tree)event_data;
    if (!t) return;

    t = TYPE_MAIN_VARIANT(t);

    if (TREE_CODE(t) != RECORD_TYPE)
        return;

    const char *nm = record_tag_name(t);
    if (nm && streq(nm, "enum_desc"))
    {
        // Now we have the REAL struct enum_desc type from the TU.
        dprintf("%s: Found type %s\n", __func__, nm);
        g_enum_desc_record = t;
        return ;
    }
}


/* ------------------------------------------------------------ */
/* Emit all recorded enums at end of unit */

static void on_finish_unit(void *, void *)
{
    // if (!g_enum_desc_record)
    //     g_enum_desc_record = find_enum_desc_record();

    if (!g_enum_desc_record)
    {
        error("struct %<enum_desc%> not found; ensure its definition is visible");
        return;
    }

    for (hash_set<tree>::iterator it = g_seen_enums.begin(); it != g_seen_enums.end(); ++it)
    {
        tree enum_type = *it;
        if (enum_type && TREE_CODE(enum_type) == ENUMERAL_TYPE)
            emit_enum_desc_for(enum_type);
    }
}


///===

static tree tree_translation_unit_decl = NULL_TREE;

static std::map<tree, tree> g_enumtype_to_descvar;

static inline bool pointer_type_p(tree t) {
  return t && TREE_CODE(t) == POINTER_TYPE;
}

static inline bool decl_name_is(tree fndecl, const char *name) {
  if (!fndecl || TREE_CODE(fndecl) != FUNCTION_DECL) return false;
  tree id = DECL_NAME(fndecl);
  if (!id || TREE_CODE(id) != IDENTIFIER_NODE) return false;
  const char *got = IDENTIFIER_POINTER(id);
  return got && streq(got, name);
}

static std::string enum_type_basename(tree enum_type) {
  std::string out;

  tree tn = TYPE_NAME(enum_type);
  if (tn) {
    if (TREE_CODE(tn) == TYPE_DECL) {
      tree id = DECL_NAME(tn);
      if (id && TREE_CODE(id) == IDENTIFIER_NODE)
        out = IDENTIFIER_POINTER(id);
    } else if (TREE_CODE(tn) == IDENTIFIER_NODE) {
      out = IDENTIFIER_POINTER(tn);
    }
  }

  if (out.empty()) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "anon_%p", (void*)enum_type);
    out = buf;
  }

  for (char &c : out) {
    if (c==' '||c==':'||c=='<'||c=='>'||c=='-'||c=='.') c = '_';
  }
  return out;
}

// Create (or reuse) a TU-scope VAR_DECL for the enum descriptor object.
// IMPORTANT: we set the VAR type to the *pointee* of wrapper return type (struct enum_desc).
static tree get_or_make_desc_var(tree enum_type, tree ret_ptr_type) {
  auto it = g_enumtype_to_descvar.find(enum_type);
  if (it != g_enumtype_to_descvar.end())
    return it->second;

  // ret_ptr_type is enum_desc_t == "struct enum_desc *"
  tree desc_type = TREE_TYPE(ret_ptr_type); // struct enum_desc
  if (!desc_type) return NULL_TREE;

  std::string nm = "__enum_desc__" + enum_type_basename(enum_type);
  tree id = get_identifier(nm.c_str());

  tree var = build_decl(BUILTINS_LOCATION, VAR_DECL, id, desc_type);
  DECL_CONTEXT(var) = tree_translation_unit_decl;

  TREE_STATIC(var) = 1;
  TREE_PUBLIC(var) = 0;        // keep it TU-local by default
  DECL_EXTERNAL(var) = 1;      // declare now; define+init later at FINISH_UNIT

  TREE_READONLY(var) = 1;      // if you plan to make it const-like
  TREE_USED(var) = 1;

  g_enumtype_to_descvar.emplace(enum_type, var);
  return var;
}

// Extract enum type from arg0 like: (enum T)0
static tree extract_enum_type_from_arg(tree arg0) {
  if (!arg0) return NULL_TREE;
  tree t = TREE_TYPE(arg0);
  if (t && TREE_CODE(t) == ENUMERAL_TYPE) return t;

  // Sometimes you get a cast node; its type is still the enum.
  // If not, give up (since you accept strict wrapper form anyway).
  return NULL_TREE;
}

// Return argument #0 of CALL_EXPR, with a safe fallback.
static tree call_arg0(tree call) {
#if defined(CALL_EXPR_ARG)
  return CALL_EXPR_ARG(call, 0);
#else
  // In many GCCs, operand 0 is callee, operand 1 is first argument.
  if (TREE_OPERAND_LENGTH(call) >= 2) return TREE_OPERAND(call, 1);
  return NULL_TREE;
#endif
}

// Rewrite "return enum_desc((enum T)0);" to "return &__enum_desc__T;"
static bool rewrite_return_enum_desc(tree fndecl) {
  tree body = DECL_SAVED_TREE(fndecl);
  if (!body) return false;

  // Wrapper return type: enum_desc_t (pointer)
  tree fn_type  = TREE_TYPE(fndecl);          // FUNCTION_TYPE
  tree ret_type = TREE_TYPE(fn_type);         // pointer type
  if (!pointer_type_p(ret_type)) return false;

  bool changed = false;

  // Tiny DFS walk that looks specifically for:
  // RETURN_EXPR ( MODIFY_EXPR ( RESULT_DECL, CALL_EXPR(...) ) )
  auto visit = [&](auto &&self, tree node) -> void {
    if (!node || changed) return;

    if (TREE_CODE(node) == RETURN_EXPR) {
      tree op0 = TREE_OPERAND(node, 0);

      // Common C GENERIC form:
      // RETURN_EXPR (MODIFY_EXPR (RESULT_DECL, <rhs>))
      if (op0 && TREE_CODE(op0) == MODIFY_EXPR) {
        tree rhs = TREE_OPERAND(op0, 1);
        if (rhs && TREE_CODE(rhs) == CALL_EXPR) {
          tree callee = CALL_EXPR_FN(rhs);
          if (callee && TREE_CODE(callee) == ADDR_EXPR)
            callee = TREE_OPERAND(callee, 0);
          if (decl_name_is(callee, kReflectFnName)) {
            tree arg0 = call_arg0(rhs);
            tree enum_type = extract_enum_type_from_arg(arg0);
            if (!enum_type) return;

            // Create/reuse the VAR_DECL for this enum.
            tree var = get_or_make_desc_var(enum_type, ret_type);
            if (!var) return;

            g_seen_enums.add(enum_type); // remember for emission later
            dprintf("%s: swap %s -> %s\n", __func__, fndecl_name_cstr(fndecl), fndecl_name_cstr(var)) ;

            // Build &var (type: pointer-to-desc_type)
            tree addr = build1(ADDR_EXPR, build_pointer_type(TREE_TYPE(var)), var);

            // Cast to wrapper return type if needed (usually same)
            if (TREE_TYPE(addr) != ret_type)
              addr = build1(NOP_EXPR, ret_type, addr);

            // Replace RHS of the MODIFY_EXPR
            TREE_OPERAND(op0, 1) = addr;
            changed = true;
            return;
          }
        }
      }
    }

    // Recurse operands
    int n = TREE_OPERAND_LENGTH(node);
    for (int i = 0; i < n; i++) {
      tree child = TREE_OPERAND(node, i);
      if (child) self(self, child);
      if (changed) return;
    }
  };

  visit(visit, body);
  return changed;
}

// Callback you register for PLUGIN_FINISH_PARSE_FUNCTION
static void on_finish_parse_function(void *event_data, void *) {
  tree fndecl = (tree)event_data;
  if (!fndecl || TREE_CODE(fndecl) != FUNCTION_DECL) return;

  // Only rewrite if the function returns a pointer (your enum_desc_t).
  tree fn_type = TREE_TYPE(fndecl);
  if (!fn_type || TREE_CODE(fn_type) != FUNCTION_TYPE) return;
  tree ret_type = TREE_TYPE(fn_type);
  if (!pointer_type_p(ret_type)) return;

  rewrite_return_enum_desc(fndecl);
}


///===

/* ------------------------------------------------------------ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    if (!plugin_default_version_check(version, &gcc_version))
        return 1;

    // Capture struct enum_desc type if visible
    register_callback(plugin_info->base_name, PLUGIN_FINISH_TYPE, on_finish_type, NULL);

    register_callback(plugin_info->base_name,
                      PLUGIN_FINISH_PARSE_FUNCTION,
                      on_finish_parse_function,
                      NULL);

    // Emit descriptors at end of TU
    register_callback(plugin_info->base_name, PLUGIN_FINISH_UNIT, on_finish_unit, NULL);

    // Install pass
    static struct register_pass_info pass_info;
    pass_info.pass = new enum_refl_pass(g);
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback(plugin_info->base_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,
                      &pass_info);

    return 0;
}