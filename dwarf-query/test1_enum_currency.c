#include <stdlib.h>

typedef struct enum_desc *enum_desc_t ;

typedef enum {
    USD = 1,
    EUR = 2,
    JPY = 3
} currency_t;

const currency_t enum_type_currency = 0;
enum_desc_t enum_desc_currency = NULL;

int main(int argc, char **argv)
{
    (void) argc ;
    (void) argv ;
}