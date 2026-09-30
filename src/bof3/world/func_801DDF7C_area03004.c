#include "bof3/world/area03004_internal.h"

/* @source 0x801DDF7C
 * @behavior AREA030 pad-mask gate: returns 0 when the shared pad state word
 * D_80145AA8 shares no bit with the caller's mask; otherwise it requests
 * frontend mode 2 through func_8014ECAC, stores 8 in byte 2 and clears byte 3
 * of the scratch work record published at the scratchpad cursor 0x1F800044,
 * and returns 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 func_801DDF7C(u16 mask) {
    if ((D_80145AA8 & mask) != 0) {
        func_8014ECAC(2);
        D_1F800044[2] = 8;
        D_1F800044[3] = 0;
        return 1;
    } else {
        return 0;
    }
}
