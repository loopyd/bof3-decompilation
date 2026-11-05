#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0C4C
 * @behavior Adds 0x20 modulo 2^32 to scratchpad work-object word 0x14 and
 * stores it unconditionally. If the new word equals 0x200, sets work byte
 * 0xB to 1, reloads the work pointer, and increments byte 2 modulo 256.
 * Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWord14By20SetByteBAndAdvanceWorkByte2At200BmagicMagic13103_801F0C4C(void) {
    u32 *work;
    u32 value;

    work = (u32 *)g_battle_work;
    value = work[5] + 0x20u;
    work[5] = value;
    if (value == 0x200u) {
        ((u8 *)work)[0xB] = 1;
        g_battle_work[2]++;
    }
}
