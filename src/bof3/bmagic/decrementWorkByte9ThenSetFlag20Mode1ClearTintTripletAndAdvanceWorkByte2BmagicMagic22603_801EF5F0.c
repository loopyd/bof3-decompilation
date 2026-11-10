#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF5F0
 * @behavior Decrements unsigned work byte 9 through the scratchpad pointer
 *           cell at 0x1F800044. When the countdown reaches zero, sets flag
 *           0x20 in byte 0, writes mode byte 0x5C to 1, clears the tint
 *           triplet at bytes 0x5D through 0x5F, and advances work byte 2.
 *           Reloads the pointer cell before each subsequent store group.
 *           Takes no arguments and returns nothing. The handler pointer at
 *           0x801F0C50 names this entry. Its 168 original bytes compare equal
 *           to emi/bmagic/magic222/03 at 0x801EF448.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSetFlag20Mode1ClearTintTripletAndAdvanceWorkByte2BmagicMagic22603_801EF5F0(void) {
    if (--g_battle_work[9] == 0) {
        g_battle_work[0] |= 0x20;
        g_battle_work[0x5C] = 1;
        g_battle_work[0x5D] = 0;
        g_battle_work[0x5E] = 0;
        g_battle_work[0x5F] = 0;
        g_battle_work[2]++;
    }
}
