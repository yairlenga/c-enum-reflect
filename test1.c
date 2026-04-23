#include <stdio.h>
enum color { C_NONE, C_RED, C_YELLOW, C_GREE } ;

void foo(enum color c) {
    printf("Color=%d\n", c) ;      // Works - enum as int,
//    printf("Color=%s\n", c) ;      // Error - Enum not a string,
}

int main(void) {
    foo(C_RED) ;
    return 0 ;
}
