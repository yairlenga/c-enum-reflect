#ifndef _ENUM_DESCX_H_
#define _ENUM_DESCX_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "enum_reflect.h"

typedef const struct enum_desc_ext *enum_desc_ext_t ;

struct enum_desc {
	const char *name ;
	uint16_t value_count ;
	uint16_t flags ;
	const enum_desc_val *values ;
	const uint16_t *lbl_off ;
	void **extra ;
	enum_desc_ext_t ext ;
	const char *lbl_str ;                // null separated list of labels + 8 nul padding.
} ;

struct enum_desc_ext {
	void *enum_cxt ;                                                        // private data, free by destroy
	void **item_cxt ;														// private per-item data, free by destroy
	void (*destroy)(enum_desc_t ed) ;
	enum_desc_idx (*find_by_value)(enum_desc_t ed, enum_desc_val value) ;
	enum_desc_idx (*find_by_label)(enum_desc_t ed, const char *label) ;
	const char *(*label_at)(enum_desc_t ed, enum_desc_idx idx) ;    		// Name by index, NULL if outside range
	enum_desc_val (*value_at)(enum_desc_t ed, enum_desc_idx idx) ;          // value by index, 0 if outside range.
	void *(*extra_at)(enum_desc_t ed, enum_desc_idx idx) ;                  // Extra handle by index, NULL if outside range.
} ;

typedef struct enum_desc_entry {
	enum_desc_val value ;
	const char *name ;
	void *extra ;
} enum_desc_entry_t ;

enum_desc_val enum_descx_value_of(enum_desc_t ed, const char *name, enum_desc_val default_value) ;
const char *enum_descx_label_of(enum_desc_t ed, enum_desc_val value, const char *default_label) ;
void *enum_descx_extra_of(enum_desc_t ed, enum_desc_val value) ;

enum_desc_idx enum_descx_find_by_value(enum_desc_t ed, enum_desc_val value) ;
enum_desc_idx enum_descx_find_by_label(enum_desc_t ed, const char *label) ;

enum_desc_val enum_descx_value_at(enum_desc_t ed, enum_desc_idx idx) ;
const char * enum_descx_label_at(enum_desc_t ed, enum_desc_idx idx) ;
void *enum_descx_extra_at(enum_desc_t ed, enum_desc_idx idx) ;

enum_desc_t enum_desc_build(const char *name, enum_desc_entry_t entries[], enum_desc_ext_t ext) ;
void enum_desc_destroy(enum_desc_t ed) ;
extern const struct enum_desc_ext enum_desc_default_ext ;

#endif