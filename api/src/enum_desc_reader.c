#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "enum_desc.h"
#include "enum_refl.h"

#define MAX_IDENT_LEN 64
#define ENUM_LOAD_MAX_ITEMS   200
#define ENUM_LOAD_MAX_TOKENS  (32 * 1024)

enum {
    ERR_NONE,
    ERR_TO_MANY_ITEMS,
    ERR_LONG_LABELS,
    ERR_IO,
};

bool get_next_line(enum_desc_source_t source)
{
    while ( true ) {

        // Reuse previous line (pushback)
        if ( source->filled ) {
            source->filled = false ;
            break ;
        }

        int result = getline(&source->line, &source->line_n, source->fp) ;
        if ( result <= 0 ) {
            // EOF or error
            if ( feof(source->fp) ) return false ;
            source->err_code = ERR_IO ;
            source->err_text = "IO Error" ;
            return false ;
        }
        // TODO: Strip comments
        bool in_quote = false ;
        int space_count = 0 ;
        char *dest = source->line ;
        int d_pos = 0 ;
        for (const char *cp = dest ; *cp ; cp++ ) {
            char ch = *cp ;
            if ( ch == '"' ) {
                in_quote = !in_quote ;
            } else if ( !in_quote ) {
                // Unquote '#' starts comment
                if ( ch == '#' ) {
                    break ;
                // collapse white space sequence
                } else if ( isspace(ch) ) {
                    space_count++ ;
                    continue ;
                }
            }
            if ( space_count ) {
                dest[d_pos++] = ' ' ;
                space_count = 0 ;
            } ;
            dest[d_pos++] = ch ;
        }
        // Only return non-blank lines.
        if ( d_pos ) {
            dest[d_pos] = 0 ;
            break ;
        }
    } ;
    return true ;
}

bool is_new_enum(const char *line, char *ident)
{
    static char work[MAX_IDENT_LEN] ;
    if ( !ident ) ident = work ;
    int s_pos = 0 ;
    return sscanf(line, " [enum.%63[A-Za-z0-9]] %n", ident, &s_pos) == 1 && !line[s_pos]  ;
}

enum_desc_t read_next_desc(enum_desc_source_t source, const char *enum_ident)
{
    // skip to next entry
    int entry_count = 0 ;
    struct enum_desc_entry entries[ENUM_LOAD_MAX_ITEMS] ;
    char labels[ENUM_LOAD_MAX_TOKENS] ;
    int labels_pos = 0 ;

    static char item_label[512] ;
    int item_val ;
    char item_extra ;
    while ( get_next_line(source) ) {

        if ( sscanf(source->line, "%63[A-Za-z0-9_] = %d *%c", item_label, &item_val, &item_extra) == 2 ) {
            if ( entry_count >= ENUM_LOAD_MAX_ITEMS ) {
                source->err_code = 1 ;
                source->err_text = "Too many items" ;
                return NULL ;
            } ;
            if ( labels_pos + strlen(item_label)+1 >= ENUM_LOAD_MAX_TOKENS) {
                source->err_code = 2 ;
                source->err_text = "Too long item labels" ;
                return NULL ;
            }
            char *label_ptr = labels + labels_pos ;
            strcpy(label_ptr, item_label) ;
            entries[entry_count] = (struct enum_desc_entry) { .name = label_ptr, .value = item_val } ;
            labels_pos += strlen(item_label)+1 ;
            entry_count++ ;
        } else if ( is_new_enum(source->line, NULL)) {
            source->filled = true ;
            break ;
        }

    }

    return enum_refl_build(enum_ident, entries, entry_count, NULL) ;
}

/* ------------------------------------------------------------ */
/* public reader                                                */
/* ------------------------------------------------------------ */

enum_desc_source_t enum_desc_source_file(FILE *fp, int lineno)
{
    enum_desc_source_t source = calloc(1, sizeof(*source)) ;
    source->fp = fp ;
    source->lineno = lineno ;
    return source ;
}

void enum_desc_source_close(enum_desc_source_t source)
{
    free(source) ;
}

enum_desc_t enum_desc_parse_next(enum_desc_source_t source) 
{
    while ( get_next_line(source) ) {
        char enum_ident[MAX_IDENT_LEN] ;
        if ( is_new_enum(source->line, enum_ident)) {
            return read_next_desc(source, enum_ident) ;
        }
    }
    return NULL ;
}
