#ifndef _ENUM_DESC_H_
#define _ENUM_DESC_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct enum_desc *enum_desc_t ;
typedef short enum_desc_idx ;
typedef const struct enum_desc_ext *enum_desc_ext_t ;

#define ENUM_DESC_NOT_FOUND ((enum_desc_idx) -1)

const char *enum_desc_label_of(enum_desc_t ed, int value) ;
bool enum_desc_parse(enum_desc_t ed, const char *label, int *value) ;

const char *enum_desc_name(enum_desc_t ed) ;
int enum_desc_item_count(enum_desc_t ed);
enum_desc_idx enum_desc_find_by_label(enum_desc_t ed, const char *label) ;
enum_desc_idx enum_desc_find_by_value(enum_desc_t ed, int value) ;
const char * enum_desc_label_at(enum_desc_t ed, enum_desc_idx idx) ;
int enum_desc_value_at(enum_desc_t ed, enum_desc_idx idx) ;
void * enum_desc_meta_at(enum_desc_t ed, enum_desc_idx idx) ;

const char *enum_desc_label_of(enum_desc_t ed, int value) ;
bool enum_desc_parse(enum_desc_t ed, const char *label, int *value) ;
int enum_desc_value_of(enum_desc_t ed, const char *label, int def_value) ;

void enum_desc_destroy(enum_desc_t ed) ;
extern const struct enum_desc_ext enum_desc_default_ext ;

// ENUM_DSC_EXTRA, or GLIBC _STDIO will expose IO functions
#ifndef ENUM_DESC_EXTRA
#define ENUM_DESC_EXTRA 1
#endif

#if ENUM_DESC_EXTRA
#include <stdio.h>
void enum_desc_print(FILE *fp, enum_desc_t ed, bool verbose) ;
#endif

extern const enum_desc_t enum_desc_null ;

#define ENUM_DESC_FUNC(tag) enum_desc_ ## tag
#define ENUM_DESC_IPARSE(tag) enum_desc_iparse_ ## tag
#define ENUM_DESC(tag) ENUM_DESC_FUNC(tag)()

#define ENUM_DESC_REQUEST(tag, enum_type) \
    const enum_type enum_type_ ## tag ; \
    const char *enum_req_ ## tag = #enum_type ; \

#define ENUM_DESC_EXTERN(tag, enum_type) \
	extern enum_desc_t ENUM_DESC_FUNC(tag)(void) ; \
    extern const char * enum_desc_label_of_ ## tag(int val) ; \
	extern bool enum_desc_parse_ ## tag(const char *label, enum_type *v) ;

#define ENUM_DESC_INLINE(tag, enum_type) \
	static inline bool ENUM_DESC_IPARSE(tag)(const char *label, enum_type *v) { \
		int val=0 ; \
		bool ok = enum_desc_parse(ENUM_DESC(tag), label, &val) ; \
		if ( ok ) *v = val ; \
		return ok ; \
	}

#define ENUM_DESCRIBE(tag, enum_type) \
	ENUM_DESC_REQUEST(tag, enum_type) \
    ENUM_DESC_EXTERN(tag, enum_type) \
	ENUM_DESC_INLINE(tag, enum_type)


#define ENUM_LABEL_OF(tag, value) \
	enum_desc_label_of(ENUM_DESC(tag), value)

#define ENUM_PARSE_LABEL(tag, label, var) \
    ENUM_DESC_IPARSE(tag)(label, var)

#ifdef __cplusplus
}
#endif

#endif

#ifndef _ENUM_DESC_DEF_H_
#define _ENUM_DESC_DEF_H_

#include <stdbool.h>
#include <stdint.h>

#include "enum_desc.h"

#ifdef _cplusplus
extern "C" {
#endif

struct enum_desc_flags {
	bool is_dynamic: 1 ;               // Set when data structure is malloc. Default: static (no free)
	bool is_custom: 1 ;                // Set if using non-standard sizes. Default (int value, uint16_t offset)
	unsigned value_sz: 2 ;             // If is_custom is true: Number of bits for the value field
	unsigned offset_sz: 2 ;            // If is_custom is true: Number of bits for the offset to the string table
} ;

/// @brief Enum description structure
struct enum_desc {
//	const char *name ;                  // Name is stored at the start of lbl_str blob, no need to duplicate it here.
	struct enum_desc_flags flags ;      // flags ;
	uint16_t item_count ;              // Number of items in the enum, also size of values[] and lbl_off[]
	const int *values ;		// Array of enum values, in declaration order.
	const uint16_t *lbl_off ;			// Array of offsets into strs for each label, in declaration order.
	void **meta ;						// Optional array of per-item metadata, in declaration order. NULL if not used.
	enum_desc_ext_t ext ;				// Optional pointer to extension struct, for dynamic descs or extra features. NULL if not used.
	const char *strs ;                  // null separated list of name, labels + 8 nul padding.
} ;

/// @brief 
struct enum_desc_ext {
	void *enum_cxt ;                                                        // private data, free by destroy
	void **item_cxt ;														// private per-item data, free by destroy
	void (*destroy)(enum_desc_t ed) ;
	enum_desc_idx (*find_by_value)(enum_desc_t ed, int value) ;
	enum_desc_idx (*find_by_label)(enum_desc_t ed, const char *label) ;
//	const char *(*label_at)(enum_desc_t ed, enum_desc_idx idx) ;    		// Name by index, NULL if outside range
//	enum_desc_val (*value_at)(enum_desc_t ed, enum_desc_idx idx) ;          // value by index, 0 if outside range.
//	void *(*extra_at)(enum_desc_t ed, enum_desc_idx idx) ;                  // Extra handle by index, NULL if outside range.
} ;

/// @brief Macro to generate enum description at compile time
/// Usage: enum_desc_t my_enum_desc = ENUM_DESC(enum my_enum)

//--------------------------------------------------------------------------------
// Implementation of enum_desc accessors
//--------------------------------------------------------------------------------

#include <string.h>

static inline const char * desc_name(enum_desc_t ed) 
{
	return ed->strs ;
}

static inline int desc_item_count(enum_desc_t ed) 
{
	return ed->item_count ;
}

static inline int value_at(enum_desc_t ed, enum_desc_idx idx) 
{
	return ed->values[idx] ;
}

static inline const char * label_at(enum_desc_t ed, enum_desc_idx idx) 
{
	return ed->strs + ed->lbl_off[idx] ;
}

static inline enum_desc_idx find_by_label(enum_desc_t ed, const char *name)
{
	int name_len_p1 = strlen(name)+1 ;
	const char *lbl_str = ed->strs ;
	for (int i=0 ; i<ed->item_count ; i++) {
		if ( !memcmp(lbl_str + ed->lbl_off[i], name, name_len_p1) ) return i ;
	}
	return ENUM_DESC_NOT_FOUND ;
}

static inline enum_desc_idx find_by_value(enum_desc_t ed, int value) 
{
	for (int i=0 ; i<ed->item_count ; i++) {
		if ( ed->values[i] == value ) return i ;
	}
	return ENUM_DESC_NOT_FOUND ;
}

static inline bool valid_index(enum_desc_t ed, enum_desc_idx idx) 
{
	return idx >=0 && idx < ed->item_count ;
}

#ifdef _cplusplus
}
#endif

#endif
