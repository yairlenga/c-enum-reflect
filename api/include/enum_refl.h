#ifndef _ENUM_REFL_H_
#define _ENUM_REFL_H_

#include "enum_desc_core.h"

#ifdef __cplusplus
extern "C" {
#endif

const char *enum_refl_name(enum_desc_t ed) ;
int enum_refl_item_count(enum_desc_t ed) ;

int enum_refl_value_of(enum_desc_t ed, const char *name, int default_value) ;
const char *enum_refl_label_of(enum_desc_t ed, int value, const char *default_label) ;
void *enum_refl_meta_of(enum_desc_t ed, int value) ;
void *enum_refl_state_of(enum_desc_t ed, int value) ;

int enum_refl_find_by_value(enum_desc_t ed, int value) ;
int enum_refl_find_by_label(enum_desc_t ed, const char *label) ;

int enum_refl_value_at(enum_desc_t ed, int idx) ;
const char * enum_refl_label_at(enum_desc_t ed, int idx) ;
const char *enum_refl_ann_at(enum_desc_t ed, int idx) ;
void *enum_refl_state_at(enum_desc_t ed, int idx) ;

struct enum_desc_entry {
	int value ;
	const char *name ;
	void *ann ;
}  ;

enum_desc_t enum_refl_build(const char *name, struct enum_desc_entry entries[], int n_entries, enum_desc_ext_t ext) ;
void enum_refl_destroy(enum_desc_t ed) ;

#if ENUM_DESC_EXTRA
typedef struct enum_desc_source {
	FILE *fp ;	
	int lineno ;
	int err_code ;
	const char *err_text ;
	bool filled ;
	size_t line_n ;
	char *line ;
} *enum_desc_source_t ;

enum_desc_source_t enum_desc_source_file(FILE *fp, int lineno) ;
void enum_desc_source_close(enum_desc_source_t source) ;
enum_desc_t enum_desc_parse_next(enum_desc_source_t source)  ;

enum_desc_t enum_repo_lookup(const char *id) ;
int enum_repo_append(FILE *fp) ;
void enum_repo_clear(void) ;
#endif

#ifdef __cplusplus
}
#endif

#endif

