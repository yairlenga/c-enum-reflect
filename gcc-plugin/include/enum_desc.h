#ifndef _ENUM_DESC_H_
#define _ENUM_DESC_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct enum_desc *enum_desc_t ;
typedef int16_t enum_desc_idx ;
typedef int enum_desc_val ;
typedef const struct enum_desc_ext *enum_desc_ext_t ;

#define ENUM_DESC_NOT_FOUND ((enum_desc_idx) -1)

const char *enum_desc_name(enum_desc_t ed) ;
int enum_desc_value_count(enum_desc_t ed);
enum_desc_idx enum_desc_find_by_label(enum_desc_t ed, const char *label) ;
enum_desc_idx enum_desc_find_by_value(enum_desc_t ed, enum_desc_val value) ;
const char * enum_desc_label_at(enum_desc_t ed, enum_desc_idx idx) ;
enum_desc_val enum_desc_value_at(enum_desc_t ed, enum_desc_idx idx) ;
void * enum_desc_meta_at(enum_desc_t ed, enum_desc_idx idx) ;

void enum_desc_destroy(enum_desc_t ed) ;
extern const struct enum_desc_ext enum_desc_default_ext ;

// ENUM_DSC_EXTRA, or GLIBC _STDIO will expose IO functions
#if !defined(ENUM_DESC_EXTRA) && defined(_STDIO_H)
#define ENUM_DESC_EXTRA 1
#endif
#if ENUM_DESC_EXTRA
#include <stdbool.h>
#include <stdio.h>
void enum_desc_print(FILE *fp, enum_desc_t ed, bool verbose) ;
#endif

extern const enum_desc_t enum_desc_null ;

#ifdef __cplusplus
}
#endif

#endif