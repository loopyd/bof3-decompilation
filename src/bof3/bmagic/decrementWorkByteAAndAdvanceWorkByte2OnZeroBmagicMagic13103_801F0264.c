#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0264
 * @behavior Decrements scratchpad work-object byte 0xA modulo 256. If the
 * new byte is zero, reloads the work pointer and increments byte 2 modulo
 * 256. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic13103_801F0264(void) {
    if (--g_battle_work[0xA] == 0) {
        g_battle_work[2]++;
    }
}
