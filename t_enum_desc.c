#include <stdio.h>

#include "enum_refl.h"

enum s1 { AAA=100, BBB, CCC, DDD, EEE, FFF=200, GGG, HHH, III, JJJ, KKK=300, LLL, MMM, NNN, OOO, PPP=400, QQQ, RRR, SSS, TTT, UUU=500, VVV, WWW, XXX, YYY, ZZZ} ;

enum s2 { VV1 = 10, VV2=20, VV3=-30, VV4=12345 } ;

static void test_static_desc(enum_desc_t ed)
{
    enum_desc_print(stdout, ed, false) ;

    for (int i=0 ; enum_desc_label_at(ed, i) ; i++) {
        const char *label = enum_desc_label_at(ed, i) ;
        int idx = enum_desc_find_by_label(ed, label) ;
        int val = enum_desc_value_at(ed, idx) ;
        int idx2 = enum_desc_find_by_value(ed, val) ;
        printf("Checked #%d: val=%d label='%s': %s\n", i, val, label, idx == i && idx2 == idx ? "PASS" : "FAIL") ;
    }

}

static void test_static_desc1(enum_desc_t ed)
{
    enum_desc_print(stdout, ed, false) ;

    printf("val(VV3)=%d\n", enum_desc_find_by_label(ed, "VV3")) ;
    printf("str(VV3)=%d\n", enum_desc_find_by_value(ed, VV3)) ;
    printf("value(-1)=%d\n", enum_desc_find_by_value(ed, -1)) ;
    printf("str(ZZZ)=%d\n", enum_desc_find_by_label(ed, "ZZZ")) ;
    printf("str(VV2)=%s\n", enum_refl_label_of(ed, VV2, "?")) ;
    printf("int(VV4)=%d\n", enum_refl_value_of(ed, "VV4", -9999));
}



#include "enum_desc_def.h"

static const struct enum_desc s1_desc = {
    .value_count = 4,
    .values = (const int[5]) { VV1, VV2, VV3, VV4, 0},
    .strs = "s2\0VV1\0VV2\0VV3\0VV4\0\0\0\0\0\0\0\0",
    .lbl_off = (const uint16_t[5]) { 4, 8, 12, 16, 20 },
} ;

#define S2_COUNT 26

static const struct enum_desc s2_desc = {
    .value_count = S2_COUNT,
    .values = (const int[S2_COUNT]) { AAA, BBB, CCC, DDD, EEE, FFF, GGG, HHH, III, JJJ, KKK, LLL, MMM, NNN, OOO, PPP, QQQ, RRR, SSS, TTT, UUU, VVV, WWW, XXX, YYY, ZZZ},
    .strs = "s1\0AAA\0BBB\0CCC\0DDD\0EEE\0FFF\0GGG\0HHH\0III\0JJJ\0KKK\0LLL\0MMM\0NNN\0OOO\0PPP\0QQQ\0RRR\0SSS\0TTT\0UUU\0VVV\0WWW\0XXX\0YYY\0ZZZ\0\0\0\0\0\0\0",
    .lbl_off = (const uint16_t[S2_COUNT+1]) { 3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63, 67, 71, 75, 79, 83, 87, 91, 95, 99, 103 },
    .meta = (void *[S2_COUNT+1]) { [5] = "Fifth", [10] = "Tenth", [20] = "Twentieth", [0] = "First", },
} ;

int main(int argc, char **argv)
{
    test_static_desc1(&s1_desc) ;
    enum_desc_destroy(&s1_desc) ;
    test_static_desc(&s2_desc) ;
    enum_desc_destroy(&s2_desc) ;
}