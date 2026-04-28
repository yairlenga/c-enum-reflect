# Automatic Enum Handling in C - Parsing, Validating and iterating.

## Quick recap - `enum` Stringification

In previous article [Automatic Enum Stringification in C via Build-Time Code Generation](https://medium.com/@yair.lenga/automatic-enum-stringification-in-c-via-build-time-code-generation-659b67133125) I described a process to extract information about enum types (list of enum identifiers and values) from debug information, allowing us to display symbolic enum labels, instead of numeric values to log files, debug messages, etc.

```c
enum color { C_NONE, C_RED, C_YELLOW, C_GREEN } ;

    // Request enum descriptor for e_color
ENUM_DESCRIBE(e_color, enum color)

void foo(enum color c) {
    printf("Color=%s(%d)\n", ENUM_LABEL_OF(e_color, c), c) ; 
}
```
The process was fully automated, relies on common tools that are already used in build pipelines, and has (practically) zero runtime cost

The next logical step will be to perform the reverse conversion - from symbolic name into a value. This functionality can be useful whenever we want to read data that is keyed into the enum - Reading input, parsing configuration files, and more.

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
    *var = c ;
    return true ;
}

int rgb[] = {
    [RED] = 0xff0000,
    [YELLOW] = 0xffff00,
    [GREEN] = 0x00ff00,
}

int show_rgb(const char *label)
{
    if ( parse_color(argv[i], &color)) {
        printf("RGB(%s)=%06x\n", argv[i], rgb[color]) ;
    } else {
        printf("Unknown Color: '%s'\n", argv[i] ;)
    }
}
```

In most cases, we will use a lookup table to make the code easier to maintain.
```c
enum color { RED, YELLOW, GREEN } ;

bool parse_color(const char *label, enum color *var)
{
    static struct { enum color c; const char *label ; } lookup = {
        { RED, "RED" },
        { YELLOW, "YELLOW" },
        { GREEN, "GREEN" },
    }
    for (int i=0 ; i<sizeof(color_lookup)/sizeof(color_lookup[0])) {
        if ( strcmp(label, lookup[i].color) == 0 ) {
                *var = lookup[i].c ;
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

The good news is that we can easily built on the foundation from the enum stringification to perform parsing !

## Parsing Strings into Enums

### The core API

The `enum_desc` module provide basic API to translate a string into the enum value. It builds on the ENUM_DESCRIBE macro which we use to create the enum descriptions.
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
No `strcmp`, no lookup tables to create, no update needed when enum values are added!

### Example: parsing values from a config file

Using the new API, we can now read a configuration files that describes the colors RGB using the color labels. 

```text
// colors.txt
RED = 0xff0000
YELLOW = 0xffff00
GREEN = 0x00ff00
```

And the code is relatively simple.

```c
enum color { RED, YELLOW, GREEN, LAST } ;

int rgb[LAST] ;

void read_rgb(FILE *fp)
{
    enum color c ;
    char color_name[30], rgb_value ;
    char line[256] ;
    while ( fgets(line, sizeof(line), fp)) {
        if ( sscanf(line "%29s = %x", color_name, value) == 2 ) {
            if ( parse_color(color_name, &c) ) {
                rgb[c] = value ;
            } else {
                fprintf(stderr, "Unable to parse: %s", line) ;
            }
        }
    }
}
```
Later, we will discuss other options to make the code more flexible - e.g., resizing to the maximum values of the enum (or the actual number of entries).

## Validation and Error Reporting
- Detecting invalid values
- Listing allowed values using iteration
- Improving error messages (user-facing / logs)

## Iterating Over Enum Values
- API: `enum_desc_item_count()`, `enum_desc_label_at()`, `enum_desc_value_at()`
- Example: dumping all enum values
- Using iteration for generic tools

## Using Enums to Introspect Data
- Enum as index into arrays / structures
- Turning enum-indexed data into self-describing output
- Example: generic dump of enum-indexed values

## Typed Helper Functions (Generated API)
- `enum_desc_parse_TAG()`
- `enum_desc_label_of_TAG()`
- Why typed wrappers improve usability
- Keeping the generic API available

## Parsing Policy: Keeping the Core Simple
- Why the default parser is strict
- Layering custom parsing logic (case-insensitive, numeric, etc.)
- Example: user-defined parsing wrapper

## Where This Leads Next
- Beyond enums: toward struct reflection
- Parsing structured data (JSON, CSV, etc.)
- Building generic tools on top of metadata

## Summary
- Minimal runtime, no duplication
- Parsing + validation “for free”
- A small step toward reflection in C
        enum color c ;
        if ( !parse_color(color_name, &c)) {
            fprintf(stderr, "Unknown Color '%s'\n", color_name) ;
            continue ;
        }
            rgb[c] = rgb_value ;
    }
}

```

## Iterating Over Enum Values

### Enumeration API

Given that the enum meta data is stored in a simple to iterate format, it's possible to iterate over all the values associated with an enum. The API provides few functions to query the enum list

The `enum_desc_item_count()`, return the number of enumerators. Each enumerator has 2 properties - label, value. The `enum_desc_label_at()` return the string (const char *) of the enumerator, and the `enum_desc_value_at()` return the integer value. Both functions checks for invalid position, and will return label=NULL, and value=0 in this case.

Both function take an enum_desc_t object. The function-like macro ENUM_DESC(tag) can be used to get the descriptor. The following code can list all the values of an enum type:
```c
enum color { NONE, BACKGROUND, FOREGROUND, HIGHLIGHT, BOLD, LAST } ;

ENUM_DESCRIBE(e_color, enum color)

void show_color_table()
{
    enum_desc_t ed = ENUM_DESC(e_color) ;
    for (int i=0, count=enum_desc_)
}
enum_
bool show_color
```
- Example: dumping all enum values
- Using iteration for generic tools

## Using Enums to Introspect Data
- Enum as index into arrays / structures
- Turning enum-indexed data into self-describing output
- Example: generic dump of enum-indexed values

## Typed Helper Functions (Generated API)
- `enum_desc_parse_TAG()`
- `enum_desc_label_of_TAG()`
- Why typed wrappers improve usability
- Keeping the generic API available

## Parsing Policy: Keeping the Core Simple
- Why the default parser is strict
- Layering custom parsing logic (case-insensitive, numeric, etc.)
- Example: user-defined parsing wrapper

## Where This Leads Next
- Beyond enums: toward struct reflection
- Parsing structured data (JSON, CSV, etc.)
- Building generic tools on top of metadata

## Summary
- Minimal runtime, no duplication
- Parsing + validation “for free”
- A small step toward reflection in C