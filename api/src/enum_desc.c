#include "enum_refl.h"
#include "enum_desc_def.h"
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

const enum_desc_t enum_desc_null = &(struct enum_desc){
	.strs = "enum_desc_null_enum\0\0\0\0\0\0\0\0",
} ;

//--------------------------------------------------------------------------------
// Implementation of enum_desc functions
//--------------------------------------------------------------------------------

enum_desc_idx enum_desc_find_by_label(enum_desc_t ed, const char *name) 
{
	return find_by_label(ed, name) ;
}

enum_desc_idx enum_desc_find_by_value(enum_desc_t ed, enum_desc_val value) 
{
	return find_by_value(ed, value) ;
}

const char * enum_desc_label_at(enum_desc_t ed, enum_desc_idx idx)
{
	if ( !valid_index(ed, idx) ) return NULL ;
	return ed->strs + ed->lbl_off[idx];
}

enum_desc_val enum_desc_value_at(enum_desc_t ed, enum_desc_idx idx)
{
	if ( !valid_index(ed, idx) ) return 0 ;
	return ed->values[idx] ;
}

void *enum_desc_meta_at(enum_desc_t ed, enum_desc_idx idx)
{
	if ( !valid_index(ed, idx) || !ed->meta ) return NULL ;
	return ed->meta[idx] ;
}

const char *enum_desc_name(enum_desc_t ed)
{
	return desc_name(ed) ;
}

int enum_desc_value_count(enum_desc_t ed)
{
	return desc_value_count(ed) ;
}

void enum_desc_destroy(enum_desc_t ed)
{
	enum_desc_ext_t ext = ed->ext ;
	if ( ext && ext->destroy ) ext->destroy(ed) ;
	if ( ed->flags & FLAG_DYNAMIC_ED ) {
		free((void *) ed->values) ;
		free((void *) ed->lbl_off) ;
		free((void *) ed->strs) ;
		free((ed->meta)) ;
		free((void *) ed) ;
	}
}

// Debug Helpers
void enum_desc_print(FILE *fp, enum_desc_t ed, bool verbose)
{
	int value_count = enum_desc_value_count(ed) ;
	int value_min = 0 ;
	int value_max = 0 ;
    fprintf(fp, "Enum '%s' %d items\n", enum_desc_name(ed), value_count) ;
    for (int i=0 ; i<value_count ; i++ ) {
		const char *item_meta = enum_desc_meta_at(ed, i) ;
		int item_val = enum_desc_value_at(ed, i) ;
		if ( !i || item_val < value_min ) value_min = item_val ;
		if ( !i || item_val > value_max ) value_max = item_val ;
		const char *item_label = enum_desc_label_at(ed, i) ;
		const char *meta_txt = verbose ? item_meta ?: "-" : item_meta ? "YES" : "NO" ;
		printf("#%d: %d (%s) meta=%s\n", i, item_val, item_label, meta_txt) ;
    }
	printf("Range: [ %d - %d ] , unused=%d\n", value_min, value_max, value_max-value_min+1-value_count) ;
}