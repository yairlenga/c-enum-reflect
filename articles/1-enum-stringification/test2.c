#include <stdio.h>
#include "enum_desc.h"

enum color { C_NONE, C_RED, C_YELLOW, C_GREE } ;

    // Request enum descriptor for e_color
ENUM_DESCRIBE(e_color, enum color)

void foo(enum color c) {
    printf("Color=%d\n", c) ;
    // print stringified label for c
    printf("Color=%s\n", ENUM_LABEL_OF(e_color, c)) ; 
    // Convenience wrapper
    printf("Color=%s\n", enum_desc_label_of_e_color(c)) ; 
}

int main(void) {
    foo(C_RED) ;
    return 0 ;
}
