#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFB5C
 * @behavior Adds two to work byte 0xB, increments byte 9, and decrements
 *           byte 0xA, in that order, at byte width. Reloads the scratchpad
 *           work pointer at 0x1F800044 before each operation and once more
 *           before comparing byte 9 with 0x28. When equal, increments byte 2
 *           through that last pointer. The table word at 0x801F08CC names
 *           this leaf handler. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByteBIncrementByte9DecrementByteAThenAdvanceByte2At28BmagicMagic16703_801EFB5C(void) {
    g_battle_work[0xB] += 2;
    g_battle_work[9]++;
    g_battle_work[0xA]--;
    if (g_battle_work[9] == 0x28) {
        g_battle_work[2]++;
    }
}
