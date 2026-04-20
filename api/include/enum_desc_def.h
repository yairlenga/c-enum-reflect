#ifndef _ENUM_DESC_DEF_H_
#define _ENUM_DESC_DEF_H_

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
	uint16_t value_count ;              // Number of items in the enum, also size of values[] and lbl_off[]
	const enum_desc_val *values ;		// Array of enum values, in declaration order.
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
	enum_desc_idx (*find_by_value)(enum_desc_t ed, enum_desc_val value) ;
	enum_desc_idx (*find_by_label)(enum_desc_t ed, const char *label) ;
//	const char *(*label_at)(enum_desc_t ed, enum_desc_idx idx) ;    		// Name by index, NULL if outside range
//	enum_desc_val (*value_at)(enum_desc_t ed, enum_desc_idx idx) ;          // value by index, 0 if outside range.
//	void *(*extra_at)(enum_desc_t ed, enum_desc_idx idx) ;                  // Extra handle by index, NULL if outside range.
} ;

/// @brief Macro to generate enum description at compile time
/// Usage: enum_desc_t my_enum_desc = ENUM_DESC(enum my_enum)
#define ENUM_DESC(T) (enum_desc_gen((T)0))

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
	return ed->value_count ;
}

static inline enum_desc_val value_at(enum_desc_t ed, enum_desc_idx idx) 
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
	for (int i=0 ; i<ed->value_count ; i++) {
		if ( !memcmp(lbl_str + ed->lbl_off[i], name, name_len_p1) ) return i ;
	}
	return ENUM_DESC_NOT_FOUND ;
}

static inline enum_desc_idx find_by_value(enum_desc_t ed, enum_desc_val value) 
{
	for (int i=0 ; i<ed->value_count ; i++) {
		if ( ed->values[i] == value ) return i ;
	}
	return ENUM_DESC_NOT_FOUND ;
}

static inline bool valid_index(enum_desc_t ed, enum_desc_idx idx) 
{
	return idx >=0 && idx < ed->value_count ;
}

#ifdef _cplusplus
}
#endif

#endif
