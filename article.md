<!-- cSpell:words OBJDIR ptype pyelftools -->
<!-- LTeX: enabled=true -->
<!-- LTeX: ignore  -->

# Automatic Enum Stringification in C via Build-Time Code Generation

## Converting enum values to labels.

If you maintain C code, you’ve probably written enum-to-string conversion functions by hand. They work—until someone adds a new enum value and forgets to update them.

When the `enum` values are assigned sequential values, it is possible to perform fast lookup with arrays, using designated initializers:
```c
enum ConnectionState {
    STATE_NONE,
    STATE_DISCONNECTED,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_ERROR,
    STATE_LAST
};

const char *connectionStateStr(enum ConnectionState state) {
    static const char *labels[] = {
        [STATE_NONE] = "STATE_NONE",
        [STATE_DISCONNECTED] = "STATE_DISCONNECTED",
        [STATE_CONNECTING] = "STATE_CONNECTING",
        [STATE_CONNECTED] = "STATE_CONNECTED",
        [STATE_ERROR] = "STATE_ERROR",
    } ;
    return state >= 0 && state < STATE_LAST ? labels[state] : NULL ;
}
```
In the other case (e.g. the `enum` values span a sparse range), you might have implemented it with a `switch` statement (or some form of lightweight hash table)
```
enum errorCode {
    E_NOT_FOUND = -1,
    E_PERMISSION = -2,
    E_OUT_OF_MEMORY = -3,
    ...
} ;

const char *errorCodeStr(enum errorCode code) {
    switch (code) {
        case E_NOT_FOUND: return "E_NOT_FOUND" ;
        case E_PERMISSION: return "E_PERMISSION" ;
        case E_OUT_OF_MEMORY: return "E_OUT_OF_MEMORY" ;
        ...
    } ;
    return NULL ;
}
```
Those lookups are commonly used to create log records, parse configuration options, and print debug output. This implementation has a few limitations:

* Requires repetitive work.
* Easy to miss updates, or introduce incorrect fixes.
* Hard to maintain if `enum` is defined in external packages. 

Languages that support reflection (`Java`, `Python`, `C#`, ...) will usually provide a stringification function, but in C, there is no built-in, standard capability.

The article discusses a lightweight solution to create stringification functions, so that you can write:
```
    printf("Connection State=%s\n", ENUM_LABEL_OF(ConnectionState, state)) ;
```
No hard-coded lookup tables. Always kept in sync with the `enum` definition at build time, using tools you already have.

## Solution - automatic stringification of `enum` values.

If `C` had reflection, we would have the option to write something like:
```
enum color { C_NONE, C_RED=2, C_YELLOW=6, C_GREEN } ;
void foo(enum color c)
{
    printf("Color=%s(%d)\n", color.to_string(c), c) ;
}
```
The *bad* news is that `C` does not provide this capability directly. The *good* news is that the compiler already has all the information needed to implement it. When code is compiled with debug (`-g`), the full definition of each (referenced) enum is captured in the object file. **We can reuse it !**

We can verify this using a debugger such as `gdb`. `gdb` will show the symbolic value of each enum variable (with `print`), and the full `enum` description with `ptype`.

```sh
(gdb) print c
$1 = C_RED
(gdb) ptype c
type = enum color {C_NONE, C_RED = 2, C_YELLOW = 6, C_GREEN}
```

### Minimal Reproducible example:

Here is a small program that demonstrates this:

```c
// color.c
#include <stdio.h>

enum color { C_NONE, C_RED=2, C_YELLOW=6, C_GREEN } ;

void foo(enum color c) {
    printf("Color=%d\n", (int) c) ;                          // print as integer
}

int main(void) {
    foo(C_RED) ;
    return 0 ;
}
```
We can compile with debug information (`gcc -c`), and inspect the enum with `gdb`:

```sh
$ gcc -g color.c
$ gdb a.out
Reading symbols from a.out...
(gdb) b foo
Breakpoint 1 at 0x1158: file color.c, line 6.
(gdb) run
Starting program: /home/user/github/articles/a.out 
Breakpoint 1, foo (c=C_RED) at color.c:6
6           printf("Color=%d\n", (int) c) ;
(gdb) print c
$1 = C_RED
(gdb) ptype c
type = enum color {C_NONE, C_RED = 2, C_YELLOW = 6, C_GREEN}
```
The information comes from the DWARF debug metadata that is embedded in the object file, and is available to the debugger (usually, it's embedded in the executable).

Instead of using this metadata only for debugging, we can extract it at build time and generate lookup tables automatically. This effectively provides reflection for enums in C — without any runtime cost.

### How it works.

The process happens entirely at build time.
1. Compile the source file with debug information. 
2. Scan the object file and extract the enum definition (via DWARF)
3. Generate a C source file containing enum descriptors.
4. Compile and link the generated code into the final binary.

```sh
gcc -c -g test.c
enum_dwarf_query --format=c test.o > test_enum.c
gcc -c -g test_enum.c
gcc -o -g prog.exe test.c test_enum.c enum_desc.c
```

The final binary contains only plain C data structures, and a small runtime support module (from enum_desc.c). There is no runtime dependency on DWARF tools or libraries, or on a debugger.

### Notes on ENUM_DESCRIBE

* The first argument (`e_color`) is a unique identifier for the descriptor.
* The second argument must resolve to a valid `enum` type, in the current translation unit.
* The enum can be defined in the same file or in an included header file.
* Important: the descriptors are generated as global symbols. Using duplicate identifier (for the same or for different enums) will result in link-time error.

You can view the definition of those macros in GitHub Gist: `enum_desc.h`. You will see that the implementation is defining multiple identifiers - all follow the pattern `enum_desc_*`.
Some identifiers have static scope, some are global objects (functions, variables). You will see those symbols if you will inspect the objects/binaries.

### Integration into CI pipeline

In practice - most of us are using build system (Make or CMake). So the generated files are rebuilt automatically when the source object file is rebuilt. This will ensure that the descriptor is up-to-date, even for enums that are defined in header files (current project, dependent objects, or system header files).

```makefile

# Assuming OBJDIR is location objects and other build artifacts are stored.
# Set 'OBJDIR = .' to work in the current folder.

$(OBJDIR)/enum_%.o: $(OBJDIR)/%.o
    enum_dwarf_query --format=c $< > $(OBJDIR)/enum_$*.c
    $(compile.c) -o $@ $(OBJDIR)/enum_$*.c

```

Note that this pipeline requires (reasonable modern) python3 runtime, including the pyelftools:
* sudo apt install python3-pyelftools
* sudo python3 -m pip install pyelftools

There is no runtime dependency on DWARF, debug information, or external tools. The binary can be fully stripped—as if nothing unusual ever happened.

### Why this approach.

Before settling on this approach, I've experimented with a few other alternatives. The main challenges were 
1. Keeping definitions in sync as enum values evolve.
2. Minimizing effort when adding new enums.
3. Maintaining single source of truth.
4. Avoiding unnecessary complexity.

The options that I've considered were
* X-Macros: Require rewriting enums into a custom format, which many codebases cannot or will not adopt.
* DSL-style (IDL files, proto): Do not work for enums defined outside your control (external libraries, system headers).
* Manual Lookup Tables: sooner or later, the mapping falls out of sync with the enums.
* Parsing Source files (AST tools, Clang toolkit, regex): Parsing C correctly is hard: anything less than a full parser is fragile, and may fail in the future.
* Compiler Plugins: Powerful, but tie the solution to single compiler/toolchain, require significant effort to develop and maintain.

While using the DWARF metadata is not perfect for all situations, it avoids the challenges of the other alternatives:
* It stays in sync with the source of truth - the way the compiler understands the `enum`.
* It requires no changes to existing source code and introduces no runtime dependency.
* It is built on tools already common in C development (gcc, clang, gdb), established and widely-used standard format (DWARF), and simple open source components (python, pyelftools)

As an extra bonus - the generated "C" code can be easily inspected/reviewed - no magic, no complex runtime, no black boxes.

## Summary

Enum stringification in C is a common problem, typically solved with manual lookup tables, or custom definitions - both have a maintenance cost and tend to drift out of sync over time. This approach addresses this problem using a different path: reusing the debug metadata already produced by the compiler to generate enum descriptors at build time.

The result is straightforward:
* No changes to existing enum definitions in the source code.
* No duplicate definitions.
* No run-time dependency on external tools or libraries.
* Always in sync with the enum as compiled.

In practice, the compiler does the heavy lifting - we just reuse it.

This is just the first step — once the metadata is available, it can also be used for parsing configuration files, validation and more. A follow-up article will explore those use cases.
