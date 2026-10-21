#include "bof3/world/area03004_internal.h"

/* @behavior initializes the shared motion state and advances the scratch
 * work-record action byte when the area gate flag is clear.
 * @source 0x801DAE3C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DAE3C(void) {
    u8* work;

    if (D_80149332 == 0) {
        D_8014932E = 0x40;
        D_8014930C = 0x3C0000;
        work = (u8*)WORLD00_AREA030_SCRATCH_PTR;
        work[3]++;
    }
}
