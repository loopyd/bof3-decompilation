#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF7D4
 * @behavior Increments scratchpad work-object byte 0xA, reloads the pointer
 * cell and increments byte 9 modulo 256. When the stored byte equals 0x18,
 * sets byte 0xB of the record pointed to by 0x801EB4E0 to one, reloads the
 * scratchpad work pointer and increments byte 2. The byte stores may alias
 * the pointer cell, so each access group reads it again. A pointer at
 * 0x801F0370 names this entry. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkBytesA9ThenSetGlobalWorkByteBToOneAndAdvanceWorkByte2At18BmagicMagic08603_801EF7D4(void) {
    u8 *workA;
    u8 *work9;
    u8 value;

    workA = g_battle_work;
    workA[0xA]++;
    work9 = g_battle_work;
    value = work9[9] + 1;
    work9[9] = value;
    if (value == 0x18) {
        D_801EB4E0[0xB] = 1;
        g_battle_work[2]++;
    }
}
