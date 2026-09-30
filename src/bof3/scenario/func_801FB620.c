#include "bof3/scenario/scena00_internal.h"

/**
 * @source 0x801FB620
 * @behavior If the state byte D_80146875 is zero, passes the scenario flag
 * word D_8014686C and flag bit 6 to func_8015B580, invokes func_801BE1B0
 * with 2, then clears the state bytes D_80146874 and D_80146875.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the state byte is reached through one
 * address taken into a local pointer, so the address lives in $s0 across both
 * calls and the final clear stores through that same base. Written plainly as
 * a global the address is reloaded per use, the prologue collapses and the
 * candidate matches only 12/23 instructions (measured with bin/asm-diff).
 */
void func_801FB620(void)
{
    u8 *state = &D_80146875;

    if (*state == 0) {
        func_8015B580(D_8014686C, 6);
        func_801BE1B0(2);
        D_80146874 = 0;
        *state = 0;
    }
}
