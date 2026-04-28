#include "enum_refl.h"
#include "enum_desc_def.h"
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------
// Implementation of enum_refl functions
//--------------------------------------------------------------------------------
const char *enum_refl_name(enum_desc_t ed) {
	return enum_desc_name(ed) ;
}

int enum_refl_item_count(enum_desc_t ed) {
	return desc_item_count(ed) ;
}

int enum_refl_find_by_value(enum_desc_t ed, int value)
{
	enum_desc_ext_t ext = ed->ext ;
	if ( ext && ext->find_by_value) return ext->find_by_value(ed, value) ;
	return find_by_value(ed, value) ;
}

int enum_refl_find_by_label(enum_desc_t ed, const char *name)
{
	enum_desc_ext_t ext = ed->ext ;
	if ( ext && ext->find_by_label ) return ext->find_by_label(ed, name) ;
	return find_by_label(ed, name) ;
}

int enum_refl_value_at(enum_desc_t ed, int idx)
{
//	enum_desc_ext_t extra = ed->ext ;
//	if ( extra && extra->label_at ) return extra->value_at(ed, idx) ;
	return valid_index(ed, idx) ? ed->values[idx] : 0 ;
}

const char * enum_refl_label_at(enum_desc_t ed, int idx)
{
//	enum_desc_ext_t extra = ed->ext ;
//	if ( extra && extra->label_at ) return extra->label_at(ed, idx) ;
	return valid_index(ed, idx) ? enum_desc_label_at(ed, idx) : NULL ;
}

void *enum_refl_meta_at(enum_desc_t ed, int idx) 
{
	return valid_index(ed, idx) && ed->meta ? ed->meta[idx] : NULL ;
}

int enum_refl_value_of(enum_desc_t ed, const char *label, int default_value)
{
	int idx = enum_refl_find_by_label(ed, label) ;
	return idx != ENUM_DESC_NOT_FOUND ? enum_desc_value_at(ed, idx) : default_value ;
}

const char *enum_refl_label_of(enum_desc_t ed, int value, const char *default_label)
{
	int idx = enum_refl_find_by_value(ed, value) ;
	return idx != ENUM_DESC_NOT_FOUND ? enum_desc_label_at(ed, idx) : default_label ;
}

void *enum_refl_state_of(enum_desc_t ed, int value)
{
	int idx = enum_refl_find_by_value(ed, value) ;
	if (idx == ENUM_DESC_NOT_FOUND ) return NULL ;
	enum_desc_ext_t ext = ed->ext ;
	void **item_cxt = ext ? ext->item_cxt : NULL ;
	if ( !item_cxt ) return NULL ;
	return item_cxt[idx] ;
}

const struct enum_desc_ext enum_desc_default_ext = {
	.find_by_label = enum_desc_find_by_label,
	.find_by_value = enum_desc_find_by_value,
//	.label_at = enum_desc_label_at,
//	.value_at = enum_desc_value_at,
} ;

static const struct enum_desc_ext enum_desc_dynamic_ext = {
	.find_by_label = enum_desc_find_by_label,
	.find_by_value = enum_desc_find_by_value,
//	.label_at = enum_desc_label_at,
//	.value_at = enum_desc_value_at,
} ;


enum_desc_t enum_refl_build(const char *name, struct enum_desc_entry entries[], int n_entries, enum_desc_ext_t ext)
{
	int count = 0 ;
	int strs_len = strlen(name)+1 ; // include enum name
	bool has_meta = false ;
	if ( n_entries < 0 ) n_entries = INT_MAX ;
	while ( count < n_entries && entries[count].name) {
		if ( entries[count].meta ) has_meta = true ;
		strs_len += strlen(entries[count].name)+1 ;
		count++ ;
	}
	strs_len+=2 ;
	char *strs = calloc(strs_len + _Alignof(max_align_t), 1) ;
	strcpy(strs, name) ;
	int off = strlen(name)+1 ;
	int *values = calloc(count+1, sizeof(*values)) ;
	uint16_t *label_off = calloc(count+1, sizeof(*label_off)) ;
	void **meta = has_meta ? calloc(count+1, sizeof(*meta)) : NULL ;

	for(int i=0; i<count ; i++ ) {
		struct enum_desc_entry *e = &entries[i] ;
		label_off[i] = off ;
		values[i] = e->value ;
		if ( meta ) meta[i] = e->meta ;
		strcpy(strs + off, e->name) ;
		off += strlen(strs+off)+1 ;
	}
	struct enum_desc *ed = calloc(1, sizeof(*ed)) ;
	*ed = (struct enum_desc) {
//		.name = strdup(name),
		.flags = { .is_dynamic = true },
		.item_count = count,
		.values = values,
		.strs = strs,
		.lbl_off = label_off,
		.meta = meta,
		.ext = ext ?: &enum_desc_dynamic_ext,
	};
	return ed ;
}

