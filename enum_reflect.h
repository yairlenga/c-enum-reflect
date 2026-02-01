#ifndef _ENUM_DESC_H_
#define _ENUM_DESC_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef const struct enum_desc *enum_desc_t ;
typedef int16_t enum_desc_idx ;
typedef int enum_desc_val ;

#define ENUM_DESC_NOT_FOUND ((enum_desc_idx) -1)

const char *enum_desc_name(enum_desc_t ed) ;
int enum_desc_value_count(enum_desc_t ed);
enum_desc_idx enum_desc_find_by_label(enum_desc_t ed, const char *label) ;
enum_desc_idx enum_desc_find_by_value(enum_desc_t ed, enum_desc_val value) ;
const char * enum_desc_label_at(enum_desc_t ed, enum_desc_idx idx) ;
enum_desc_val enum_desc_value_at(enum_desc_t ed, enum_desc_idx idx) ;

const char *enum_desc_label_of(enum_desc_t ed, enum_desc_val value, const char *default_label) ;
enum_desc_val enum_desc_value_of(enum_desc_t ed, const char *label, enum_desc_val default_value) ;
void *enum_desc_extra_of(enum_desc_t ed, enum_desc_val value) ;

#endif