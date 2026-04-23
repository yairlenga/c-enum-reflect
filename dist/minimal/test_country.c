#include "country.h"

#include "enum_desc.h"

#include <stdio.h>
#include <stdlib.h>

ENUM_DESCRIBE(country3, country_t)

int main(int argc, char **argv)
{
    for (int i=1 ; i<argc ; i++) {
        country_t country_code = atoi(argv[i]) ;
        if ( country_code ) {
            printf("CCY(%d)=%s\n", country_code, ENUM_LABEL_OF(country3, country_code)) ;
        }
    }
}
