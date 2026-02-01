#ifndef _ENUM_REFL_H_
#define _ENUM_REFL_H_

#include "enum_desc.h"

enum_desc_val enum_refl_value_of(enum_desc_t ed, const char *name, enum_desc_val default_value) ;
const char *enum_refl_label_of(enum_desc_t ed, enum_desc_val value, const char *default_label) ;
void *enum_refl_meta_of(enum_desc_t ed, enum_desc_val value) ;
void *enum_refl_state_of(enum_desc_t ed, enum_desc_val value) ;

enum_desc_idx enum_refl_find_by_value(enum_desc_t ed, enum_desc_val value) ;
enum_desc_idx enum_refl_find_by_label(enum_desc_t ed, const char *label) ;

enum_desc_val enum_refl_value_at(enum_desc_t ed, enum_desc_idx idx) ;
const char * enum_refl_label_at(enum_desc_t ed, enum_desc_idx idx) ;
void *enum_refl_meta_at(enum_desc_t ed, enum_desc_idx idx) ;
void *enum_refl_state_at(enum_desc_t ed, enum_desc_idx idx) ;

struct enum_desc_entry {
	enum_desc_val value ;
	const char *name ;
	void *meta ;
}  ;

enum_desc_t enum_refl_build(const char *name, struct enum_desc_entry entries[], enum_desc_ext_t ext) ;


#endif