# C Enum-to-String: Build-Time Generation from Compiler Debug Info

## Using `enum` for variables with closed set of values.

In `C` (and `C++`), we use `enum` when we have variable that can have a value from a closed set of related choices. Common use cases are

Representing a state:
```c
enum ConnectionState {
    STATE_DISCONNECTED,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_ERROR
};
```
Return Codes:
```c
enum Result {
    RES_OK,
    RES_NOT_FOUND,
    RES_TIMEOUT,
    RES_INVALID
};
```
Grouping of related constants:
```c
enum Direction {
        DIR_NORTH,
        DIR_SOUTH,
        DIR_WEST,
        DIR_EAST
}
```
Technically, each `enum` has a set of enumerators - each one has a distinct identifier, and is assigned integer value (developers can control the value assignment if needed). We will refer to them as the "labels" and "values".

In all those cases, using `enum` improve the code. We can use the enumerator identifier in assignment, conditions, pass as parameter. As an extra bonus, when we use a debugger (e.g., `gdb`), we can see the enumerator identifier when we print `enum` variable, and the complete definition of the `enum` (with `ptype`).

## The problem: displaying `enum` values in output, log files and debug printout.

The bad news - if we want to print the `enum` variable in text output - we can only display the enumerator value. Languages that support reflection (`Java`, `Python`, `C#`, ...) will usually provide a stringification function, but in C:
```c
#include <stdio.h>
enum color { C_NONE, C_RED, C_YELLOW, C_GREE } ;

void foo(enum color c) {
    printf("Color=%d\n", c) ;      // Works - enum as int,
    printf("Color=%s\n", c) ;      // Error - Enum not a string,
}

int main(void) {
    foo(C_RED) ;
    return 0 ;
}
```
Will result in compile error:
```sh
x.c:6:20: error: format ‘%s’ expects argument of type ‘char *’, but argument 2 has type ‘unsigned int’ [-Werror=format=]
    6 |     printf("Color=%s\n", c) ;
      |                   ~^     ~
      |                    |     |
      |                    |     unsigned int
      |                    char *
      |                   %d
```
Removing this line will result in getting the numeric value of the passed color
```
Color=1
```

To address the problem, many libraries provide built-in stringification functions for key `enum` values. Common examples are standard C library `strerror*`, which can convert `errno` to text, but NOT to E_XXX identifier (For legacy reasons, the `errno` is not defined as enum - instead it is implemented as `int` + series of `#define`). Similar function exists in `libcurl`: `curl_easy_strerror(rc)`, which returns error string, but NOT the `CURLE_XXX` enumerator identifier.

## Solution - automatic stringification of `enum` values.

If `C` had reflection, we would have the option to write something:
```
enum color { C_NONE, C_RED=2, C_YELLOW=6, C_GREEN } ;
void foo(enum color c)
{
    printf("Color=%s(%d)\n", color_to_string(c), c) ;
}
```

The good news is that it relatively simple to auto-generate the "color_to_string" function. We already know that gdb has the ability to show the enumeration name of a variable, and the full enum definition. For the above example, we can verify it with:
```sh
gcc -g color.c
gdb a.out
Reading symbols from a.out...
(gdb) b foo
Breakpoint 1 at 0x1158: file x.c, line 6.
(gdb) run
Starting program: /home/user/github/articles/2026-fast-exp/a.out 
Breakpoint 1, foo (c=C_RED) at x.c:6
6           printf("Color=%d\n", (int) c) ;
(gdb) print c
$1 = C_RED
(gdb) ptype c
type = enum color {C_NONE, C_RED = 2, C_YELLOW = 6, C_GREEN}
```

Therefore, we can write a small utility to extract the enum description from the object file (more details: [Wikipedia `dwarf`](https://en.wikipedia.org/wiki/DWARF)). Our code now looks like:

```c
#include <stdio.h>

    // Single header file for all definitions for enum_desc
#include "enum_desc.h"

enum color { C_NONE, C_RED, C_YELLOW, C_GREE } ;

    // The ENUM_DESCRIBE request enum_descriptor `e_color`, based on the
    // type 'enum color'. The identifier must be globally unique.

ENUM_DESCRIBE(e_color, enum color)

void foo(enum color c) {
    printf("Color=%d\n", c) ;                          // print as integer

    // The ENUM_LABEL_OF return the stringified value of a value
    // Based on previous defined enum_descriptor.
    printf("Color=%s\n", ENUM_LABEL_OF(e_color, c)) ;  // print as a string

}

int main(void) {
    foo(C_RED) ;
    return 0 ;
}
```
Some explanations:

The `ENUM_DESCRIBE` takes two parameters - tag and type. The tag is a global identifier that will reference the `enum` description. Because the `enum` description will be exposed as global, it is an error for two modules the request the same tag to be exposed - it will result in link error - duplicate symbols.

The `type` must resolve to an enum type that is recognized inside this compilation unit. This can be:
* An `enum` defined in a header file that is included.
* Locally defined `enum` in this file.
* Locally define `enum` in this function.

### How will it work.

The 'magic' will happen in phases:

1. The code that has references to `enum` descriptors (ENUM_DESCRIBE(...)) must be compiled with debug mode (-g). 
2. The object file will be scanned by a python module which will generate the `C` file that will hold the `enum` descriptors.
3. The newly generated `C` file will be compiled.
4. The final executable should link the generated object file from step 3, and the support module (single c file).

Minimal example: one source file (test.c). The functionality is implemented in 2 files: a header file 'enum_desc.h' and a source file 'enum_desc.c' 

```sh
gcc -c -g test.c
enum_dwarf_query --format=c test.o > test_enum.c
gcc -c -g test_enum.c
gcc -o prog.exe test.c test_enum.c enum_desc.c
```

Most C programs are built in `make` or `CMake`. In this case, the build script should rebuild the generated source, whenever the object file is updated. From a practical point of view, better to organize the calls to the enum description code into small number of files - to avoid time consuming scan after each build.

In Makefile:
```
# Binary
PROG = prog.exe

# List of all source files
SRCS = file1.c file2.c file3.c enum_file2.c enum_desc.c

enum_%.o: %.o
    enum_dwarf_query --format=c $< > enum_$*.c
    $(COMPILE.c) -o $@ enum_$*.c

$(PROG): $(SRCS:%.c=%.o)
    $(LINK.c) -o $@ $^

```



More realistic example - using the "libcurl"

Will print
The problem: labels, not numbers
Functions return enum values — but displaying or logging them gives raw integers. Why that matters in real code.
§ 1.1 — A motivating example
Function returns a Status enum. Caller prints it, gets 2. Unhelpful in logs, useless to users.
§ 1.2 — A second domain
Same problem in a different context — e.g. Direction or ErrorCode. The pattern is general, not a special case.
§ 1.3 — The naive fix and why it hurts
A hand-written switch or string array. Works — until someone adds an enum member and forgets to update it. Sets up sections 2 and 4.
§ 2
Generating the conversion table automatically
Use build-time tooling to extract enum names from debug info and emit C stubs — no manual maintenance.
§ 2.1 — The approach: read DWARF, emit C
A small script (Python / awk / shell) reads the compiled object's DWARF debug info and writes a .c file with a string table and lookup function per enum.
§ 2.2 — A minimal Makefile integration
The rule: compile the header to an object, run the generator, compile the stub, link everything. Show the 3–4 Makefile lines.
§ 2.3 — Sample generated output
Show what the emitted .c looks like — the string array, the bounds check, the function signature. Reader should see it's ordinary C.
§ 3
Parsing: symbolic input → enum value
The inverse direction: a user types "ERROR" in a config file or CLI flag — we need the matching constant.
§ 3.1 — Linear scan with strcmp
Loop the same string table from §2, strcmp each entry, return the index. One table, two directions.
§ 3.2 — Handling no-match
Return -1, a sentinel, or a dedicated UNKNOWN member. Tradeoffs for each.
§ 3.3 — Case sensitivity
Should "error" match "ERROR"? strcasecmp or normalize the input first. Config files usually want case-insensitive.
§ 4
Comparing the alternatives
Other approaches exist — each with real costs. The goal is to show why DWARF is the right foundation.
§ 4.1 — Hand-coded lookup tables
Works fine until the enum grows. Every new member is a silent maintenance obligation. The mismatch is undetectable at compile time without extra tooling.
§ 4.2 — Source code scanning
Parse the header with regex or a C parser. Fragile: macros, attributes, conditional compilation, and multi-file enums all break naive scanners. Correct parsing requires a full preprocessor.
§ 4.3 — Compiler extensions
GCC/Clang plugins or __attribute__ tricks can emit names, but they're compiler-specific, complex to set up, and non-portable.
§ 4.4 — DWARF: the right level of abstraction
Already emitted by every major compiler with -g. Standardized format. Macros and attributes are already resolved. Enum names, values, and types are all present — no guessing.
§ 4.5 — BTF as a future direction
BPF Type Format is a compact, runtime-accessible type system gaining traction in the Linux kernel ecosystem. Same idea as DWARF but designed for live introspection — worth watching.
§ 5
Takeaway, limitations, and extensions
What to take away, and where the approach needs more thought before you use it blindly.
§ 5.1 — Core takeaway
DWARF gives you the truth the compiler already knows. Generate once at build time, use in both directions. No maintenance, no drift.
§ 5.2 — Bitmask enums vs. value enums
Flags enums (READ | WRITE) don't map to a single name. The generator needs a different output strategy — decompose into set bits, emit a list.
§ 5.3 — Synonyms and duplicate values
C allows two enumerators with the same numeric value. DWARF records both. Which name wins on output? Which is accepted on input? Policy decision — document it.
§ 5.4 — Further extensions
Stripping common prefixes (STATUS_OK → "OK"). Generating JSON or YAML tables. Extending to structs. Next article territory.