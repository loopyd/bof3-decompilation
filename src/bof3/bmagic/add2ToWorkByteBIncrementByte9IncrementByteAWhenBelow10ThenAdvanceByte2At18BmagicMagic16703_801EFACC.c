#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFACC
 * @behavior Adds two to work byte 0xB, increments byte 9, and increments
 *           byte 0xA only while it is below 0x10, in that order. Reloads
 *           the scratchpad work pointer at 0x1F800044 before each group
 *           and after storing byte 0xA. When byte 9 is 0x18, increments
 *           byte 2 through the last pointer. The handler-table word at
 *           0x801F08C8 names this frameless leaf. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByteBIncrementByte9IncrementByteAWhenBelow10ThenAdvanceByte2At18BmagicMagic16703_801EFACC(void) {
    g_battle_work[0xB] += 2;
    g_battle_work[9]++;
    if (g_battle_work[0xA] < 0x10) {
        g_battle_work[0xA]++;
    }
    if (g_battle_work[9] == 0x18) {
        g_battle_work[2]++;
    }
}
