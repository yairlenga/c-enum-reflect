#include <stdio.h>
#include "enum_desc.h"

enum color { C_NONE, C_RED, C_YELLOW, C_GREEN } ;

    // Request enum descriptor for e_color
ENUM_DESCRIBE(e_color, enum color)

void foo(enum color c) {
    printf("Color(%d)=%s\n", (int) c, ENUM_LABEL_OF(e_color, c)) ; 
}

int main(void) {
    foo(C_RED) ;
    foo(C_YELLOW) ;
    foo(C_GREEN) ;
    return 0 ;
}
