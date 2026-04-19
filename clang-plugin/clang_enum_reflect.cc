// enum_desc_clang_inject.cpp
//
// Clang frontend plugin that injects enum_desc instances
// directly into the AST (no extra generated files).
//
// Focuses on:
//   - value_count
//   - values[]
//   - lbl_off[]
//   - strs blob
//
// Everything is emitted into the object file.
//
// Build as a Clang plugin (.so) and load with -Xclang -load.

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>
#include <string>
#include <cctype>

using namespace clang;

namespace {

// -----------------------------------------------------------------------------
// Configuration – must match your ABI
// -----------------------------------------------------------------------------

static constexpr const char *ENUM_DESC_STRUCT = "enum_desc";
static constexpr const char *ENUM_DESC_VAL_STRUCT = "enum_desc_val";
static constexpr const char *ENUM_DESC_VAL_VALUE_FIELD = "value";

// enum_desc fields
static constexpr const char *F_value_count = "value_count";
static constexpr const char *F_flags       = "flags";
static constexpr const char *F_values      = "values";
static constexpr const char *F_lbl_off     = "lbl_off";
static constexpr const char *F_meta        = "meta";
static constexpr const char *F_ext         = "ext";
static constexpr const char *F_strs        = "strs";

// -----------------------------------------------------------------------------
// Utilities
// -----------------------------------------------------------------------------

static std::string sanitize(std::string s) {
  for (char &c : s)
    if (!std::isalnum((unsigned char)c) && c != '_')
      c = '_';
  if (s.empty() || std::isdigit((unsigned char)s[0]))
    s = "_" + s;
  return s;
}

static bool hasAnnotate(const Decl *D, llvm::StringRef tag) {
  for (auto *A : D->specific_attrs<AnnotateAttr>())
    if (A->getAnnotation() == tag)
      return true;
  return false;
}

static const RecordDecl *findStruct(ASTContext &Ctx, llvm::StringRef name) {
  for (Decl *D : Ctx.getTranslationUnitDecl()->decls())
    if (auto *RD = dyn_cast<RecordDecl>(D))
      if (RD->isStruct() && RD->getIdentifier() && RD->getName() == name)
        return RD;
  return nullptr;
}

static const FieldDecl *findField(const RecordDecl *RD, llvm::StringRef name) {
  for (auto *F : RD->fields())
    if (F->getIdentifier() && F->getName() == name)
      return F;
  return nullptr;
}

static Expr *u16(ASTContext &Ctx, unsigned v, SourceLocation L) {
  return IntegerLiteral::Create(
      Ctx, llvm::APInt(16, v), Ctx.UnsignedShortTy, L);
}

static Expr *i64(ASTContext &Ctx, long long v, SourceLocation L) {
  return IntegerLiteral::Create(
      Ctx, llvm::APInt(64, v, true), Ctx.LongLongTy, L);
}

static Expr *nullOf(ASTContext &Ctx, QualType T, SourceLocation L) {
  auto *Z = IntegerLiteral::Create(
      Ctx, llvm::APInt(32, 0), Ctx.IntTy, L);
  return ImplicitCastExpr::Create(
      Ctx, T, CK_IntegralCast, Z, nullptr, VK_PRValue, FPOptionsOverride());
}

static Expr *blobStr(ASTContext &Ctx, llvm::StringRef data, SourceLocation L) {
  QualType CharTy = Ctx.CharTy;
  QualType ArrTy =
      Ctx.getConstantArrayType(CharTy,
                               llvm::APInt(32, data.size() + 1),
                               nullptr,
                               ArraySizeModifier::Normal,
                               0);

  auto *SL = StringLiteral::Create(
      Ctx, data, StringLiteralKind::Ordinary,
      false, ArrTy, L);

  QualType PtrTy = Ctx.getPointerType(Ctx.getConstType(CharTy));
  return ImplicitCastExpr::Create(
      Ctx, PtrTy, CK_ArrayToPointerDecay,
      SL, nullptr, VK_PRValue, FPOptionsOverride());
}

static VarDecl *addStatic(ASTContext &Ctx,
                          TranslationUnitDecl *TU,
                          llvm::StringRef name,
                          QualType T,
                          Expr *init,
                          SourceLocation L) {
  auto *VD = VarDecl::Create(
      Ctx, TU, L, L, &Ctx.Idents.get(name),
      T, nullptr, SC_Static);
  VD->setInit(init);
  VD->setIsUsed();
  TU->addDecl(VD);
  return VD;
}

// -----------------------------------------------------------------------------
// Enum collection
// -----------------------------------------------------------------------------

struct EnumItem {
  std::string label;
  long long value;
};

struct EnumInfo {
  std::string rawName;
  std::string name;
  std::vector<EnumItem> items;
  SourceLocation loc;
};

class EnumVisitor : public RecursiveASTVisitor<EnumVisitor> {
public:
  EnumVisitor(ASTContext &Ctx) : Ctx(Ctx), SM(Ctx.getSourceManager()) {}

  bool VisitEnumDecl(EnumDecl *ED) {
    if (!ED || !ED->isCompleteDefinition())
      return true;
    if (!SM.isWrittenInMainFile(SM.getSpellingLoc(ED->getLocation())))
      return true;
    if (!hasAnnotate(ED, "enum_desc"))
      return true;

    EnumInfo E;
    E.loc = ED->getLocation();
    E.rawName = ED->getIdentifier()
                  ? ED->getNameAsString()
                  : "anon_enum";
    E.name = sanitize(E.rawName);

    for (auto *C : ED->enumerators()) {
      llvm::APSInt v = C->getInitVal();
      E.items.push_back({
        C->getNameAsString(),
        v.isSigned() ? v.getSExtValue() : (long long)v.getZExtValue()
      });
    }

    Enums.push_back(std::move(E));
    return true;
  }

  std::vector<EnumInfo> Enums;

private:
  ASTContext &Ctx;
  SourceManager &SM;
};

// -----------------------------------------------------------------------------
// AST Consumer
// -----------------------------------------------------------------------------

class EnumDescConsumer : public ASTConsumer {
public:
  void HandleTranslationUnit(ASTContext &Ctx) override {
    EnumVisitor V(Ctx);
    V.TraverseDecl(Ctx.getTranslationUnitDecl());
    if (V.Enums.empty())
      return;

    const RecordDecl *RD_desc = findStruct(Ctx, ENUM_DESC_STRUCT);
    const RecordDecl *RD_val  = findStruct(Ctx, ENUM_DESC_VAL_STRUCT);
    if (!RD_desc || !RD_val) {
      llvm::errs() << "enum_desc structs not visible in this TU\n";
      return;
    }

    static constexpr const char *kF_values = "values" ;
    static constexpr const char *kF_lbl_off = "lbl_off";
    static constexpr const char *kF_strs    = "strs"   ;
    static constexpr const char *kF_meta     = "meta"   ;
    static constexpr const char *kF_ext      = "ext"    ;

    const FieldDecl *F_val_value = findField(RD_val, ENUM_DESC_VAL_VALUE_FIELD);
    const FieldDecl *F_values   = findField(RD_desc, kF_values);
    const FieldDecl *F_lbl_off  = findField(RD_desc, kF_lbl_off);
    const FieldDecl *F_strs     = findField(RD_desc, kF_strs);
    const FieldDecl *F_meta    = findField(RD_desc, kF_meta);
    const FieldDecl *F_ext     = findField(RD_desc, kF_ext);

    TranslationUnitDecl *TU = Ctx.getTranslationUnitDecl();
    QualType QT_val  = Ctx.getRecordType(RD_val);
    QualType QT_desc = Ctx.getRecordType(RD_desc);

    for (const auto &E : V.Enums) {
      unsigned N = E.items.size();
      SourceLocation L = E.loc;

      // Build strs blob
      std::string blob = E.rawName;
      blob.push_back('\0');

      std::vector<unsigned> offs;
      for (auto &it : E.items) {
        offs.push_back(blob.size());
        blob += it.label;
        blob.push_back('\0');
      }
      blob.append(8, '\0');

      Expr *Strs = blobStr(Ctx, blob, L);

      // values[]
      std::vector<Expr*> vals;
      for (auto &it : E.items) {
        auto *IL = new (Ctx) InitListExpr(
            Ctx, L, { i64(Ctx, it.value, L) }, L);
        IL->setType(QT_val);
        vals.push_back(IL);
      }

      QualType ArrValTy =
          Ctx.getConstantArrayType(
              Ctx.getConstType(QT_val),
              llvm::APInt(32, N),
              nullptr, ArraySizeModifier::Normal, 0);

      auto *ValsInit = new (Ctx) InitListExpr(Ctx, L, vals, L);
      ValsInit->setType(ArrValTy);

      VarDecl *VD_vals =
          addStatic(Ctx, TU,
                    "__enum_desc_vals_" + E.name,
                    ArrValTy, ValsInit, L);

      // lbl_off[]
      std::vector<Expr*> offsInit;
      for (unsigned o : offs)
        offsInit.push_back(u16(Ctx, o, L));

      QualType ArrOffTy =
          Ctx.getConstantArrayType(
              Ctx.UnsignedShortTy,
              llvm::APInt(32, N),
              nullptr, ArraySizeModifier::Normal, 0);

      auto *OffInit = new (Ctx) InitListExpr(Ctx, L, offsInit, L);
      OffInit->setType(ArrOffTy);

      VarDecl *VD_off =
          addStatic(Ctx, TU,
                    "__enum_desc_lbl_off_" + E.name,
                    ArrOffTy, OffInit, L);

      // enum_desc
      Expr *ValsPtr =
          ImplicitCastExpr::Create(
              Ctx, F_values->getType(), CK_ArrayToPointerDecay,
              DeclRefExpr::Create(Ctx, {}, L, VD_vals,
                                  false, L, VD_vals->getType(), VK_LValue),
              nullptr, VK_PRValue, FPOptionsOverride());

      Expr *OffPtr =
          ImplicitCastExpr::Create(
              Ctx, F_lbl_off->getType(), CK_ArrayToPointerDecay,
              DeclRefExpr::Create(Ctx, {}, L, VD_off,
                                  false, L, VD_off->getType(), VK_LValue),
              nullptr, VK_PRValue, FPOptionsOverride());

      std::vector<Expr*> D = {
        u16(Ctx, N, L),
        u16(Ctx, 0, L),
        ValsPtr,
        OffPtr,
        nullOf(Ctx, F_meta->getType(), L),
        nullOf(Ctx, F_ext->getType(), L),
        Strs
      };

      auto *DI = new (Ctx) InitListExpr(Ctx, L, D, L);
      DI->setType(Ctx.getConstType(QT_desc));

      addStatic(Ctx, TU,
                "__enum_desc_" + E.name,
                Ctx.getConstType(QT_desc),
                DI, L);
    }
  }
};

// -----------------------------------------------------------------------------
// Plugin registration
// -----------------------------------------------------------------------------

class EnumDescAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &, llvm::StringRef) override {
  llvm::errs() << "[enum-desc] CreateASTConsumer()\n";
    return std::make_unique<EnumDescConsumer>();
  }

  bool ParseArgs(const CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }

  ActionType getActionType() override {
    return AddAfterMainAction;
  }
};

} // namespace

static FrontendPluginRegistry::Add<EnumDescAction>
X("enum-desc-inject", "Inject enum_desc into the TU");
