# Automatic Enum Handling in C - Parsing, Validating and iterating.

## Quick recap - `enum` Stringification

In previous article [Automatic Enum Stringification in C via Build-Time Code Generation](https://medium.com/@yair.lenga/automatic-enum-stringification-in-c-via-build-time-code-generation-659b67133125), I described a process to extract information about enum types (the list of enum identifiers and values) from debug (DWARF) information. This allows us to display symbolic enum labels instead of numeric values in logs, debug output, and more.

```c
enum color { C_NONE, C_RED, C_YELLOW, C_GREEN } ;

    // Request enum descriptor for e_color
ENUM_DESCRIBE(e_color, enum color)

void foo(enum color c) {
    printf("Color=%s(%d)\n", ENUM_LABEL_OF(e_color, c), c) ; 
}
```
The process is fully automated, relies on common tools that are already used in build pipelines, and has (practically) zero runtime cost.

## Parsing Strings into Enums

The next logical step is the reverse conversion - from symbolic name to value. This is useful when reading external input keyed by enum values: command-line arguments, configuration files, or user input.

Languages that support reflection (`Java`, `Python`, `C#`, ...) will usually provide lookup functions (`EnumClass.valueOf(String)`, `Enum.Parse(...)`), but in C, there is no built-in, standard capability. 

This article discusses a lightweight solution to parse strings into enum values, so that you can write parsing functions:

```c
ENUM_DESCRIBE(e_color, enum color)

bool parse_color(const char *label, enum color *var)
{
    return ENUM_PARSE_TYPE(e_color, label, var) ;
}
```

No hard-coded lookup tables. Always kept in sync with the `enum` definition at build time, using tools you already have.

> In practice, this means enum labels can be part of the input interface, not just internal constants.

### Quick Start

Download the latest minimal package (~20KB):

https://github.com/yairlenga/c-enum-reflect/releases/latest

See the [Releases](https://github.com/yairlenga/c-enum-reflect/releases/) page for other versions and packages.

### Example: Reading input

Let's assume we have in memory information about each color (e.g., RGB values) and we want to allow the user to choose the RGB by color name, which is an enum. Currently, we will write a lookup table, or inline a list of strcmp to map the input label into the correct enum index.

```c
enum color { RED, YELLOW, GREEN } ;

bool parse_color(const char *label, enum color *var)
{
    if ( strcmp(label, "RED") == 0) *var = RED ;
    else if ( strcmp(label, "YELLOW") == 0) *var = YELLOW ;
    else if ( strcmp(label, "GREEN") == 0) *var = GREEN ;
    else return false ;

    return true ;
}

int rgb[] = {
    [RED] = 0xff0000,
    [YELLOW] = 0xffff00,
    [GREEN] = 0x00ff00,
} ;

int show_rgb(const char *label)
{
    enum color c ;
    if ( parse_color(label, &c)) {
        printf("RGB(%s)=%06x\n", label, rgb[c]) ;
    } else {
        printf("Unknown Color: '%s'\n", label) ;
    }
}
```

In most cases, we will use a lookup table to make the code easier to maintain.
```c
enum color { RED, YELLOW, GREEN } ;

bool parse_color(const char *label, enum color *var)
{
    static struct { enum color c; const char *label ; } color_lookup[] = {
        { RED, "RED" },
        { YELLOW, "YELLOW" },
        { GREEN, "GREEN" },
    } ;
    for (int i=0 ; i<sizeof(color_lookup)/sizeof(color_lookup[0]) ; i++) {
        if ( strcmp(label, color_lookup[i].label) == 0 ) {
                *var = color_lookup[i].c ;
                return true ;
        }
    }
    return false ;
}
```

Either way, we will have the same problem that we had with enum stringification:
* Requires repetitive work.
* Easy to miss updates, or introduce incorrect fixes.
* Hard to maintain if `enum` is defined in external packages.

The good news is that we can build directly on the existing enum metadata to support parsing.

### API for Parsing Enum

The `enum_desc` module provides a basic API to translate a string into the enum value. It builds on the ENUM_DESCRIBE macro which we use to create the enum descriptions.
```
// Define a tag that reference the enum
ENUM_DESCRIBE(tag, enum_type)

// Function-like macro.
// If the label matches one of the enum labels:
//    store the matching enum value into the variable that p_enum points to.
//    return true
// On error:
//    does NOT modify the variable pointed to by p_enum
//    return false
bool ENUM_PARSE_TYPE(enum_tag, const char *label, enum_type *p_enum) ;
```

Our parsing function is now simple, short:
```c
ENUM_DESCRIBE(e_color, enum color)

bool parse_color(const char *label, enum color *var)
{
    return ENUM_PARSE_TYPE(e_color, label, var) ;
}
```
No `strcmp`, no lookup tables to create, and no updates needed when enum values change.

### Example: parsing values from a config file

Using the new API, we can now read a configuration file that describes RGB values for colors, leveraging the symbolic names.

```text
// colors.txt
FOREGROUND = 0xff0000
BACKGROUND = 0xffff00
HIGHLIGHT = 0x00ff00
BOLD  = 0xff0000
```

And the code is relatively simple.

```c
enum color { FOREGROUND, BACKGROUND, HIGHLIGHT, BOLD, LAST } ;

int rgb[LAST] ;

void read_rgb(FILE *fp)
{
    char line[256] ;
    while ( fgets(line, sizeof(line), fp)) {
        enum color c ;
        char color_name[30] ;
        int rgb_value ;
        if ( sscanf(line, "%29s = %x", color_name, &rgb_value) == 2 &&
            parse_color(color_name, &c) ) {
                rgb[c] = rgb_value ;
        }
    }
}
```
Later, we will discuss other options to make the code more flexible - e.g., resizing to the maximum values of the enum (or the actual number of entries).

## Validation and Error Reporting

When the `ENUM_PARSE_TYPE()` call fail to match the label, it will return `false`, and will **not** modify the enum variable. This can be used to introduce defaults, and error logging as needed. For example:

```c
enum color { FOREGROUND, BACKGROUND, HIGHLIGHT, BOLD, LAST } ;

int rgb[LAST] = {
    [FOREGROUND] = 0x000000,  // Black
    [BACKGROUND] = 0xffffff,  // White
    [HIGHLIGHT] = 0xffff00,   // Yellow
    [BOLD] = 0xff0000,        // Red
} ;

ENUM_DESCRIBE(e_color, enum color)

bool parse_color(const char *label, enum color *var)
{
    return ENUM_PARSE_TYPE(e_color, label, var) ;
}

void read_rgb(FILE *fp)
{
    char line[256] ;
    while ( fgets(line, sizeof(line), fp)) {
        char color_name[30] ;
        int rgb_value ;
        if ( sscanf(line, "%29s = %x", color_name, &rgb_value) != 2 ) {
            fprintf(stderr, "Bad Config line: '%s'\n", line) ;
            continue ;
        }
        enum color c ;
        if ( !parse_color(color_name, &c) ) {
            fprintf(stderr, "Unknown color: '%s'\n", color_name) ;
            continue ;
        }
        rgb[c] = rgb_value ;
    }
}
```

Once enum metadata is available, parsing is not the only operation we can support. We can also iterate over all enum values, enabling generic processing and introspection.

## Iterating Over Enum Values

### Enumeration API

Given that the enum metadata is stored in a simple to iterate format, it's possible to iterate over all the values of a single enum. The API provides a few functions to query the enum list

* The `enum_desc_item_count()`, returns the number of enumerators.
* The `enum_desc_label_at()` returns the label (const char *) of the enumerator, based on position. Return NULL on bad position.
* The `enum_desc_value_at()` returns the integer value (int) of the enumerator, based on position. Return 0 on bad position.
  
Both function take an enum_desc_t object. The function-like macro ENUM_DESC(tag) can be used to get the descriptor. The following code can list all the enumerators of an enum type:
```c
enum color { NONE, BACKGROUND, FOREGROUND, HIGHLIGHT, BOLD, LAST } ;

ENUM_DESCRIBE(e_color, enum color)

    // Showing all colors, using count
void show_color_enum()
{
    enum_desc_t ed = ENUM_DESC(e_color) ;
    for (int i=0, count=enum_desc_item_count(ed) ; i<count ; i++) {
        printf("Color #%d: %s = %d\n", i, enum_desc_label_at(ed, i), enum_desc_value_at(ed, i)) ;
    }
}

```

### Dumping all enum values

The reflection API can work on any `enum` - the above code is generic. The `enum_desc_print` dump all the data about an enum in human readable format:

```c
// ISO country codes.
enum country3 {
    ...
    ISO3_BGR = 100, ISO3_MMR = 104, ISO3_BDI = 108,
    ISO3_BLR = 112, ISO3_KHM = 116, ISO3_CMR = 120,
    ISO3_CAN = 124, ISO3_CPV = 132, ISO3_CYM = 136,
    ...
}
```
Will print:
```text
Enum 'country3' 191 items, dynamic=TRUE, custom=FALSE, offset_sz=2, value_sz=4
...
#12: 44 (ISO3_BHS) meta=-
#13: 48 (ISO3_BHR) meta=-
#14: 50 (ISO3_BGD) meta=-
#15: 51 (ISO3_ARM) meta=-
#16: 52 (ISO3_BRB) meta=-
#17: 56 (ISO3_BEL) meta=-
#18: 60 (ISO3_BMU) meta=-
...
Range: [ 4 - 894 ] , unused=700
```

### Using iteration for generic tools

The iteration API can also be used to provide custom behavior. For example, to lookup for enumerator using case insensitive match. This will allow referencing the enum using uppercase, lowercase of mixed-case.

```c
bool enum_parse_case_cmp(enum_desc_t ed, const char *label, int *var)
{
    int count = enum_desc_item_count(ed) ;
    for (int i=0 ; i<count ; i++) {
        if ( strcasecmp(label, enum_desc_label_at(ed, i)) == 0) {
            *var = enum_desc_value_at(ed, i) ;
            return true ;
        }
    }
    return false ;
}
```


## Summary

- Enum metadata is generated automatically from debug (DWARF) information.
- The same data supports parsing, validation, and iteration.
- No manual lookup tables or duplicated definitions.
- Always in sync with the compiled enum — including external libraries.
- A small step toward practical reflection in C.

### Small Notes

In addition to the generic API, typed helper functions such as
`enum_desc_label_of_color()` and `enum_desc_parse_color()` can be generated for convenience.
These wrappers improve type safety and readability, but are not required —
the generic API is sufficient in most cases.

### Disclaimer

The examples and benchmarks in this article, including linked code snippets, are simplified and reconstructed for illustration purposes. They are not taken from any production system, and do not reflect the design or implementation of any specific codebase.

This is a personal approach based on general experience working with C codebases. It does not represent any official guideline or the opinion of my employer.

As with any low-level technique, evaluate carefully before adopting it in production.

### Usage and License

The supporting files (`enum_desc.c`, `enum_desc.h`, `enum_dwarf_query.py`) are provided under the MIT license and are intended to be copied and used as-is in your own projects.

You can simply copy and/or modify them into your project and integrate the extractor into your build process - no special packaging or setup is required
