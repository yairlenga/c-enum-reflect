#include "enum_refl.h"
#include "enum_desc_def.h"
#include <stdbool.h>
#include <stddef.h>

// Implementation of enum_desc functions

#define FLAG_DYNAMIC_ED (1<<0)

enum_desc_idx enum_desc_find_by_label(enum_desc_t ed, const char *name) 
{
	int name_len_p1 = strlen(name)+1 ;
	for (int i=0 ; i<ed->value_count ; i++) {
		if ( !memcmp(ed->lbl_str + ed->lbl_off[i], name, name_len_p1) ) return i ;
	}
	return ENUM_DESC_NOT_FOUND ;
}

enum_desc_idx enum_desc_find_by_value(enum_desc_t ed, enum_desc_val value) 
{
	for (int i=0 ; i<ed->value_count ; i++) {
		if ( ed->values[i] == value ) return i ;
	}
	return ENUM_DESC_NOT_FOUND ;
}

const char * enum_desc_label_at(enum_desc_t ed, enum_desc_idx idx)
{
	if ( idx < 0 || idx >= ed->value_count ) return NULL ;
	return ed->lbl_str + ed->lbl_off[idx];
}

enum_desc_val enum_desc_value_at(enum_desc_t ed, enum_desc_idx idx)
{
	if ( idx < 0 || idx >= ed->value_count ) return 0 ;
	return ed->values[idx] ;
}

void *enum_desc_meta_at(enum_desc_t ed, enum_desc_idx idx)
{
	if ( idx < 0 || idx >= ed->value_count || !ed->meta ) return NULL ;
	return ed->meta[idx] ;
}

const char *enum_desc_name(enum_desc_t ed)
{
	return ed->name ;
}

int enum_desc_value_count(enum_desc_t ed)
{
	return ed->value_count ;
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

enum_desc_idx enum_refl_find_by_value(enum_desc_t ed, enum_desc_val value)
{
	enum_desc_ext_t ext = ed->ext ;
	if ( ext && ext->find_by_value) return ext->find_by_value(ed, value) ;
	return enum_desc_find_by_value(ed, value) ;
}

enum_desc_val enum_refl_value_at(enum_desc_t ed, enum_desc_idx idx)
{
//	enum_desc_ext_t extra = ed->ext ;
//	if ( extra && extra->label_at ) return extra->value_at(ed, idx) ;
	return enum_desc_value_at(ed, idx) ;
}


const char * enum_refl_label_at(enum_desc_t ed, enum_desc_idx idx)
{
//	enum_desc_ext_t extra = ed->ext ;
//	if ( extra && extra->label_at ) return extra->label_at(ed, idx) ;
	return enum_desc_label_at(ed, idx) ;
}

enum_desc_idx enum_refl_find_by_label(enum_desc_t ed, const char *name)
{
	enum_desc_ext_t ext = ed->ext ;
	if ( ext && ext->find_by_label ) return ext->find_by_label(ed, name) ;
	return enum_desc_find_by_label(ed, name) ;
}

void *enum_refl_meta_at(enum_desc_t ed, enum_desc_idx idx) 
{
	return enum_desc_meta_at(ed, idx) ;
}

enum_desc_val enum_refl_value_of(enum_desc_t ed, const char *label, enum_desc_val default_value)
{
	int idx = enum_refl_find_by_label(ed, label) ;
	return enum_desc_value_at(ed, idx) ;
}

const char *enum_refl_label_of(enum_desc_t ed, enum_desc_val value, const char *default_label)
{
	int idx = enum_refl_find_by_value(ed, value) ;
	return idx >= 0 ? enum_desc_label_at(ed, idx) : default_label ;
}

void *enum_refl_state_of(enum_desc_t ed, enum_desc_val value)
{
	int idx = enum_refl_find_by_value(ed, value) ;
	if  ( idx < 0 || idx >= ed->value_count ) return NULL ;
	void **item_cxt = ed->ext && ed->ext->item_cxt ;
	if ( !item_cxt ) return NULL ;
	return item_cxt[idx] ;
}

enum_desc_t enum_refl_build(const char *name, struct enum_desc_entry entries[], enum_desc_ext_t ext)
{
	int count = 0 ;
	int strs_len = 0 ;
	bool has_meta = false ;
	while ( entries[count].name) {
		if ( entries[count].meta ) has_meta = true ;
		strs_len += strlen(entries[count].name)+1 ;
		count++ ;
	}
	strs_len+=2 ;
	int off = 1 ;
	char *strs = calloc(strs_len + _Alignof(max_align_t), 1) ;
	enum_desc_val *values = calloc(count+1, sizeof(*values)) ;
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
		.name = name,
		.flags = FLAG_DYNAMIC_ED,
		.value_count = count,
		.values = values,
		.lbl_str = strs,
		.lbl_off = label_off,
		.meta = meta,
		.ext = ext ?: &enum_desc_dynamic_ext,
	};
	return ed ;
}

void enum_desc_destroy(enum_desc_t ed)
{
	enum_desc_ext_t ext = ed->ext ;
	if ( ext && ext->destroy ) ext->destroy(ed) ;
	if ( ed->flags && FLAG_DYNAMIC_ED ) {
		free((void *) ed->values) ;
		free((void *) ed->lbl_off) ;
		free((void *) ed->lbl_str) ;
		free((ed->meta)) ;
		free((void *) ed) ;
	}
}
