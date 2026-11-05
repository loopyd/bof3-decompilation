#include "bof3/battle/battle03_internal.h"

/* @source 0x801E75B0
 * @behavior Starts EMI stream slot 0x23C/0x23E when work halfword +0x2C is
 * zero, or 0x23D/0x23F otherwise, selecting the first slot when work byte
 * +0x08 is below two. Then increments scratchpad work byte +0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startWorkVariantEmiStreamAndAdvanceByte1(void) {
    Battle03LocalWork* work;
    u32 slot;

    work = D_801EB4E0;
    if (FIELD_REF(u16, work, 0x2cu) == 0) {
        if (work->unk_08 < 2) {
            slot = 0x23cu;
        } else {
            slot = 0x23eu;
        }
    } else {
        if (work->unk_08 < 2) {
            slot = 0x23du;
        } else {
            slot = 0x23fu;
        }
    }
    func_80161FDC(slot);
    SPAD_PTR_SLOT(u8, 0x44u)[1]++;
}
