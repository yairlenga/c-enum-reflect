#include "enum_refl.h"

int main(int argc, char **argv)
{
	(void) argc ;
	(void) argv ;
	enum_desc_source_t source = enum_desc_source_file(stdin, 0) ;

	for ( ;; ) {
		enum_desc_t ed = enum_desc_parse_next(source) ;
		if ( !ed ) {
			if ( !source->err_code ) break ;
			fprintf(stderr, "%s: Parse error %d (%s) on line %d\n", __func__, source->err_code, source->err_text, source->lineno) ;
			continue ;
		} ;
		enum_desc_print(stdout, ed, true) ;
	}
}
