#include "bof3/battle/battle03_internal.h"

/* @source 0x801E915C
 * @behavior Clamps the panel state halfword at +0x6 to 0xC8 and advances byte
 * +0x3 when it is below 0xC9, otherwise subtracts 4 from the state, then
 * forwards both signed panel halfwords to func_801D750C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E915C(void) {
    u8* panel = (u8*)D_80148648;
    s16 state = FIELD_REF(s16, panel, 6);

    if (state < 0xC9) {
        FIELD_REF(s16, panel, 6) = 0xC8;
        FIELD_REF(u8, panel, 3)++;
    } else {
        FIELD_REF(s16, panel, 6) = state - 4;
    }
    func_801D750C(*(s16*)&D_80148648[4], *(s16*)&D_80148648[6]);
}
