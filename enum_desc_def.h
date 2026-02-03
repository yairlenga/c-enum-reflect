#ifndef _ENUM_DESC_DEF_H_
#define _ENUM_DESC_DEF_H_

#include "enum_desc.h"

/// @brief Enum description structure
struct enum_desc {
//	const char *name ;                  // Name is stored at the start of lbl_str blob, no need to duplicate it here.
	uint16_t value_count ;              // Number of items in the enum, also size of values[] and lbl_off[]
	uint16_t flags ;			        // bitfield of flags, for internal use. 
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

#endif