#include "bof3/battle/battle03_internal.h"

/* @source 0x801DD704
 * @behavior Returns zero when the eligibility helper succeeds; otherwise tests selected flag-record bit 0x400.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 testFlag400WhenEligible(u8 arg0) {
    if (func_801DB524(arg0) == 0) {
        return ((D_80145FB8[arg0].flags_00 & 0x400) + 1) <= 1;
    }

    return 0;
}
