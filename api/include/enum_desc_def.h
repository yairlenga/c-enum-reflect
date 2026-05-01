#ifndef _ENUM_DESC_DEF_H_
#define _ENUM_DESC_DEF_H_

#include <stdint.h>
#include <stdbool.h>

#include "enum_desc_core.h"

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
//	uint16_t name_off ;                // offset to enum name
	uint32_t enum_ann ;                // offset to enum annotations
	struct enum_desc_flags flags ;     // flags ;
	uint16_t item_count ;              // Number of items in the enum, also size of values[] and lbl_off[]
	const int *values ;		           // Array of enum values, in declaration order.
	const uint16_t *lbl_off ;          // Array of offsets into strs for each label, in declaration order.
	const uint32_t *ann_off ;          // Annotation offsets
	enum_desc_ext_t ext ;              // Optional pointer to extension struct, for dynamic descs or extra features. NULL if not used.
	const char *strs ;                 // null separated list of name, labels + 8 nul padding.
} ;

/// @brief 
struct enum_desc_ext {
	void *enum_ext ;                                                        // private data, free by destroy
	void **item_ext ;														// private per-item data, free by destroy
	void (*destroy)(enum_desc_t ed) ;
	int (*find_by_value)(enum_desc_t ed, int value) ;
	int (*find_by_label)(enum_desc_t ed, const char *label) ;
//	const char *(*label_at)(enum_desc_t ed, int idx) ;    		// Name by index, NULL if outside range
//	enum_desc_val (*value_at)(enum_desc_t ed, int idx) ;          // value by index, 0 if outside range.
//	void *(*extra_at)(enum_desc_t ed, int idx) ;                  // Extra handle by index, NULL if outside range.
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

static inline int value_at(enum_desc_t ed, int idx) 
{
	return ed->values[idx] ;
}

static inline const char * label_at(enum_desc_t ed, int idx) 
{
	return ed->strs + ed->lbl_off[idx] ;
}

static inline int find_by_label(enum_desc_t ed, const char *name)
{
	int name_len_p1 = strlen(name)+1 ;
	for (int i=0 ; i<ed->item_count ; i++) {
		if ( !memcmp(name, label_at(ed, i), name_len_p1) ) return i ;
	}
	return ENUM_DESC_NOT_FOUND ;
}

static inline int find_by_value(enum_desc_t ed, int value) 
{
	for (int i=0 ; i<ed->item_count ; i++) {
		if ( ed->values[i] == value ) return i ;
	}
	return ENUM_DESC_NOT_FOUND ;
}

static inline bool valid_index(enum_desc_t ed, int idx) 
{
	return idx >=0 && idx < ed->item_count ;
}

#ifdef _cplusplus
}
#endif

#endif
