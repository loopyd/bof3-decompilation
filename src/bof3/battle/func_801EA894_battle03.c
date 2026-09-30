#include "bof3/battle/battle03_internal.h"

/* @source 0x801EA894
 * @behavior Raises the panel update flag, then advances the panel state: when the
 * state halfword at +0x6 is 0x12 it increments byte +0x3, otherwise it adds 8 to
 * the state; finally it redispatches the panel handler.
 * @status partial
 * @match 75.00
 * @residual Allocator residual: only the register pair for the state halfword and the
 * flag constant differs (original `li/lh v1/v0`, current `v0/v1`); 24/24 instructions,
 * 96/96 bytes, first mismatch +0x000C.
 */
void func_801EA894(void) {
    u8* panel = (u8*)D_80148648;
    s16 state;

    D_801EC2E4 = 1;
    state = FIELD_REF(s16, panel, 6);
    if (state == 0x12) {
        FIELD_REF(u8, panel, 3)++;
    } else {
        FIELD_REF(s16, panel, 6) = state + 8;
    }
    func_801EAAB8();
}
