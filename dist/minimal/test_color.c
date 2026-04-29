#include <stdio.h>
#include "enum_desc.h"

enum color { C_NONE, C_RED, C_YELLOW, C_GREEN } ;

    // Request enum descriptor for e_color
ENUM_DESCRIBE(e_color, enum color)

void show_color(enum color c) {
    printf("Color(%d)=%s\n", (int) c, ENUM_LABEL_OF(e_color, c)) ; 
}

void parse_color(const char *label)
{
    enum color c = C_NONE ;
    if ( ENUM_PARSE_LABEL(e_color, label, &c)) {
        printf("Color '%s' is %d\n", label, c) ;
    } else {
        printf("No Color '%s'\n", label) ;
    }
}

int main(void) {
    show_color(C_RED) ;
    show_color(C_YELLOW) ;
    show_color(C_GREEN) ;

    parse_color("C_RED") ;
    parse_color("C_YELLOW") ;
    parse_color("C_GREEN") ;
    parse_color("C_BAD") ;
}
