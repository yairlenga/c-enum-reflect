#ifndef _ENUM_DESC_H_
#define _ENUM_DESC_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct enum_desc *enum_desc_t ;
typedef const struct enum_desc_ext *enum_desc_ext_t ;

#define ENUM_DESC_NOT_FOUND (-1)

const char *enum_desc_label_of(enum_desc_t ed, int value) ;
bool enum_desc_parse(enum_desc_t ed, const char *label, int *value) ;

const char *enum_desc_name(enum_desc_t ed) ;
int enum_desc_item_count(enum_desc_t ed);
int enum_desc_find_by_label(enum_desc_t ed, const char *label) ;
int enum_desc_find_by_value(enum_desc_t ed, int value) ;
const char * enum_desc_label_at(enum_desc_t ed, int idx) ;
int enum_desc_value_at(enum_desc_t ed, int idx) ;
void * enum_desc_meta_at(enum_desc_t ed, int idx) ;

const char *enum_desc_label_of(enum_desc_t ed, int value) ;
bool enum_desc_parse(enum_desc_t ed, const char *label, int *value) ;

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
#define ENUM_DESC(tag) ENUM_DESC_FUNC(tag)()

#define ENUM_REQUEST(tag, enum_type) \
    static const enum_type enum_type_ ## tag ; \
    static const char *enum_req_ ## tag = #enum_type ; \
	extern enum_desc_t ENUM_DESC_FUNC(tag)(void) ; \

#define ENUM_DESCRIBE(tag, enum_type) \
	ENUM_REQUEST(tag, enum_type) \
    extern const char * enum_desc_label_of_ ## tag(int val) ; \
	extern bool enum_desc_parse_ ## tag(const char *label, enum_type *v) ;

#define ENUM_LABEL_OF(tag, value) \
    enum_desc_label_of(ENUM_DESC(tag), value)

#define ENUM_PARSE_TYPE(tag, label, var) \
    enum_desc_parse(ENUM_DESC(tag), label, var)

#ifdef __cplusplus
}
#endif

#endif