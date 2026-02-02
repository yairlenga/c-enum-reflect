#include <stdio.h>

#include "enum_refl.h"

enum e1 { E1=1, E3=3, E100=100} ;

enum s2 { VV1 = 10, VV2=20, VV3=-30, VV4=12345 } ;

static void show_desc(enum_desc_t ed)
{
    int value_count = enum_desc_value_count(ed) ;
    printf("Enum '%s' %d items\n", enum_desc_name(ed), value_count) ;
    for (int i=0 ; i<value_count ; i++ ) {
        printf("#%d: %d (%s)\n", i, enum_desc_value_at(ed, i), enum_desc_label_at(ed, i)) ;
    }
}

static void test_static_desc(enum_desc_t ed)
{
    show_desc(ed) ;

    printf("val(VV3)=%d\n", enum_desc_find_by_label(ed, "VV3")) ;
    printf("str(VV3)=%d\n", enum_desc_find_by_value(ed, VV3)) ;
    printf("value(-1)=%d\n", enum_desc_find_by_value(ed, -1)) ;
    printf("str(ZZZ)=%d\n", enum_desc_find_by_label(ed, "ZZZ")) ;
    printf("str(VV2)=%s\n", enum_refl_label_of(ed, VV2, "?")) ;
    printf("int(VV4)=%d\n", enum_refl_value_of(ed, "VV4", -9999));
}

#include "enum_desc_def.h"

static const struct enum_desc s2_desc = {
    .value_count = 4,
    .name = "s2",
    .values = (const int[5]) { VV1, VV2, VV3, VV4, 0},
    .lbl_str = "\0VV1\0VV2\0VV3\0VV4\0\0\0\0\0\0\0\0",
    .lbl_off = (const uint16_t[5]) { 1, 5, 9, 13, 17 },
} ;



static void test_dynamic_refl(void)
{
    enum_desc_t e1_desc = enum_refl_build("e1", (struct enum_desc_entry []) { { E1, "E1"}, { E3, "E3" }, { E100, "E100"}, {} }, NULL) ;

    show_desc(e1_desc) ;
    {
        enum_desc_t ed = e1_desc ;
        printf("str(E3)=%d\n", enum_refl_find_by_label(ed, "E3")) ;
        printf("val(E100)=%d\n", enum_refl_find_by_value(ed, E100)) ;
        printf("val(-1)=%d\n", enum_refl_find_by_value(ed, -1)) ;
        printf("str(ZZZ)=%d\n", enum_refl_find_by_label(ed, "ZZZ")) ;
        printf("str(VV2)=%s\n", enum_refl_label_of(ed, VV2, "?")) ;
        printf("int(VV4)=%d\n", enum_refl_value_of(ed, "VV4", -9999));
    }
    enum_desc_destroy(e1_desc) ;
}


int main(int argc, char **argv)
{
    test_static_desc(&s2_desc) ;
    test_dynamic_refl() ;
}