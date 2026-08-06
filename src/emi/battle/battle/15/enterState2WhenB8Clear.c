#include "internal.h"

/* @source 0x800A5120 */
// @behavior When D_801485B8 bit 0 is clear, sets state byte 0xE3 to 2, clears 0xE4, and clears bit 7 of 0xE5.
void enterState2WhenB8Clear(void) {
    u8 *state;

    if (!(D_801485B8 & 1)) {
        state = &D_801462E5;
        D_801462E3 = 2;
        D_801462E4 = 0;
        *state &= 0x7F;
    }
}
