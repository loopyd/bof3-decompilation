#include "bof3/scenario/scena00_internal.h"

/* @source 0x801FC818
 * @behavior When the route word D_80143F00 is two, the argument's high half
 * selects kind 0x2B and flag bit 6 of D_8014686C is set, requests primary state
 * 7 by storing D_80146874 = 7 and clears the secondary sub-state D_80146875;
 * when D_80143F00 is 0x18 with D_80143F03 clear, kind 0x14 and flag bit 7 of
 * D_8014686C clear, runs func_8015C088, requests primary state 6 the same way
 * and returns 1; otherwise returns 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 seedPrimaryStateOnRecordCallback(s32 packed_kind) {
    if (D_80143F00 == 2 && (packed_kind >> 16) == 0x2B &&
        func_8015B5D4(D_8014686C, 6) != 0) {
        D_80146874 = 7;
        D_80146875 = 0;
    }

    if (D_80143F00 == 0x18 && D_80143F03 == 0 && (packed_kind >> 16) == 0x14 &&
        func_8015B5D4(D_8014686C, 7) == 0) {
        func_8015C088();
        D_80146874 = 6;
        D_80146875 = 0;
        return 1;
    }

    return 0;
}
