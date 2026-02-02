// t_gcc.c
#include <stdio.h>
#include "enum_desc_def.h"
enum currency { USD=840, EUR=978, JPY=826, GBP=826, AUD=36 } ;

static enum_desc_t enum_desc_gen(enum currency x)
{
    printf("stub: %s (%d)\n", __func__, x) ;
    return NULL ;
}

enum_desc_t currency_desc(enum currency c) { return enum_desc_gen(c); }

int main(int argc, char **argv)
{
	enum_desc_t foo = currency_desc((enum currency) 0) ;
	printf("foo(%d)\n", foo->value_count) ;
}
