#ifndef _ENUM_DESC_DEF_H_
#define _ENUM_DESC_DEF_H_

#include "enum_desc.h"

/// @brief Enum description structure
struct enum_desc {
	const char *name ;
	uint16_t value_count ;
	uint16_t flags ;
	const enum_desc_val *values ;
	const uint16_t *lbl_off ;
	void **meta ;
	enum_desc_ext_t ext ;
	const char *lbl_str ;                // null separated list of labels + 8 nul padding.
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