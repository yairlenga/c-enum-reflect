#include "enum_refl.h"
#include "enum_desc_def.h"

#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

const enum_desc_t enum_desc_null = &(struct enum_desc){
	.strs = "enum_desc_null_enum\0\0\0\0\0\0\0\0",
} ;

//--------------------------------------------------------------------------------
// Implementation of enum_desc functions
//--------------------------------------------------------------------------------

int enum_desc_find_by_label(enum_desc_t ed, const char *name) 
{
	return find_by_label(ed, name) ;
}

int enum_desc_find_by_value(enum_desc_t ed, int value) 
{
	return find_by_value(ed, value) ;
}

const char * enum_desc_label_at(enum_desc_t ed, int idx)
{
	if ( !valid_index(ed, idx) ) return NULL ;
	return label_at(ed, idx) ;
}

int enum_desc_value_at(enum_desc_t ed, int idx)
{
	if ( !valid_index(ed, idx) ) return 0 ;
	return value_at(ed, idx) ;
}

const char *enum_desc_ann_at(enum_desc_t ed, int idx)
{
	if ( !valid_index(ed, idx) || !ed->ann_off ) return NULL ;
	int ann_off = ed->ann_off[idx] ;
	return ann_off ? ed->strs+ann_off : NULL ;
}

const char *enum_desc_name(enum_desc_t ed)
{
	return desc_name(ed) ;
}

const char *enum_desc_ann(enum_desc_t ed)
{
	return ed->ann_off ? ed->strs + ed->enum_ann : NULL ;
}

int enum_desc_item_count(enum_desc_t ed)
{
	return desc_item_count(ed) ;
}

const char *enum_desc_label_of(enum_desc_t ed, int value)
{
	int idx = find_by_value(ed, value) ;
	if ( idx == ENUM_DESC_NOT_FOUND ) return NULL ;
	return label_at(ed, idx) ;
}

bool enum_desc_parse(enum_desc_t ed, const char *label, int *value)
{
	int idx = find_by_label(ed, label) ;
	if ( idx == ENUM_DESC_NOT_FOUND ) return false ;
	*value = value_at(ed, idx) ;
	return true ;
}

int enum_desc_value_of(enum_desc_t ed, const char *label, int def_value)
{
	enum_desc_parse(ed, label, &def_value) ;
	return def_value ;
}

void enum_desc_destroy(enum_desc_t ed)
{
	enum_desc_ext_t ext = ed->ext ;
	if ( ext && ext->destroy ) ext->destroy(ed) ;
	if ( ed->flags.is_dynamic ) {
		free((void *) ed->values) ;
		free((void *) ed->lbl_off) ;
		free((void *) ed->strs) ;
		free((void *) ed->ann_off);
		free((void *) ed) ;
	}
}

// Debug Helpers

void enum_desc_print(FILE *fp, enum_desc_t ed, bool verbose)
{
	int item_count = enum_desc_item_count(ed) ;
	int value_min = 0 ;
	int value_max = 0 ;
	const char *ann = enum_desc_ann(ed) ;
	if ( !ann ) ann = verbose ? "-" : "NO" ;

	fprintf(fp, "Enum '%s' %d items, dynamic=%s, custom=%s, offset_sz=%d, value_sz=%d, ann=%s\n", enum_desc_name(ed), item_count,
		ed->flags.is_dynamic ? "TRUE" : "FALSE",
		ed->flags.is_custom ? "TRUE" : "FALSE",
		(int) (ed->flags.is_custom ? 2 << ed->flags.offset_sz : (int) sizeof(*ed->lbl_off)),
		(int) ed->flags.is_custom ? 2 << ed->flags.value_sz : (int) sizeof(*ed->values),
		enum_desc_ann(ed)) ;

    for (int i=0 ; i<item_count ; i++ ) {
		int item_val = enum_desc_value_at(ed, i) ;
		if ( !i || item_val < value_min ) value_min = item_val ;
		if ( !i || item_val > value_max ) value_max = item_val ;
		const char *item_label = enum_desc_label_at(ed, i) ;
		const char *item_ann = enum_desc_ann_at(ed, i) ;
		if ( !item_ann ) item_ann = verbose ? "-" : "NO" ;
		printf("#%d: %d (%s) ann=%s\n", i, item_val, item_label, item_ann) ;
    }
	printf("Range: [ %d - %d ], unused=%d\n", value_min, value_max, value_max-value_min+1-item_count) ;
}