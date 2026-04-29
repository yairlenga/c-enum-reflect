# enum-desc (minimal)

Small, dependency-light runtime for working with C enums as data.

This package provides:
- value → label lookup  
- label → value lookup  
- iteration over enum entries  

It is designed to work with code generated from DWARF or other enum sources.

---

## Contents

```
enum_desc.h
enum_desc.c
enum_dwarf_query
```

---

## Quick example

```c
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
        printf("No Color '%s'\n", c) ;
    }
}

int main(void) {
    show_color(C_RED) ;
    show_color(C_YELLOW) ;
    show_color(C_GREEN) ;

    parse_color("RED") ;
    parse_color("YELLOW") ;
    parse_color("GREEN") ;
    parse_color("BAD") ;
}

```
---

## Build

```sh
# Compile the source file
cc -g -I .   -c -o test2.o test2.c
# Extract enum definition from object file.
./enum_dwarf_query --format=c test2.o > ./enum_test2.c
# Compile the enum definintions
cc -g -I .   -c -o gen_test2.o ./enum_test2.c
# Compile the runtime library
cc -g -I .   -c -o enum_desc.o enum_desc.c
# Link
cc -g -I .    -o test2 test2.o enum_desc.o gen_test2.o
```

## Test programs

Compile the generated source together with your code:

```
make
```

It will build 2 small test programs
* test2 - basic test - on simple enum (RED, YELLOW, GREEN).
* test_country - larger enum, ~200 ISO 4217 country codes.

Execute the tests
```shell
$ make test
./test2
Color(1)=C_RED
Color(2)=C_YELLOW
Color(3)=C_GREEN
./test_country 840 826 392 250 276
CCY(840)=ISO3_USA
CCY(826)=ISO3_GBR
CCY(392)=ISO3_JPN
CCY(250)=ISO3_FRA
CCY(276)=ISO3_DEU
```
---

## Generating enum data

The `enum_dwarf_query` tool can extract enums from compiled objects:

```bash
gcc -g -c file.o
enum_dwarf_query file.o --format=c > gen_file.c
```

You can then compile the generated `.c` file together with this runtime.

---

## Notes

- Generated files (`gen_*.c`) should not be edited manually  
- No external dependencies are required  
- This is the minimal runtime; additional features may exist in the full package  

---

## License

The files in this folder are provided under the MIT license and are intended to be copied and used as-is in your own projects.
