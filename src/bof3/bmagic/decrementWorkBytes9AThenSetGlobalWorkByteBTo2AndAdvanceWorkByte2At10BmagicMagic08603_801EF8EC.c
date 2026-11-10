#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF8EC
 * @behavior Decrements scratchpad work-object byte 9, reloads the work
 * pointer and decrements byte 0xA modulo 256, then reloads the pointer again.
 * When byte 9 equals 0x10, sets byte 0xB of the record pointed to by
 * 0x801EB4E0 to two, reloads the scratchpad work pointer and increments byte
 * 2. The byte stores may alias the pointer cell, so each access group reads
 * it again. A pointer at 0x801F037C names this entry. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkBytes9AThenSetGlobalWorkByteBTo2AndAdvanceWorkByte2At10BmagicMagic08603_801EF8EC(void) {
    u8 *work9;
    u8 *workA;
    u8 *state;

    work9 = g_battle_work;
    work9[9]--;
    workA = g_battle_work;
    workA[0xA]--;
    state = g_battle_work;
    if (state[9] == 0x10) {
        D_801EB4E0[0xB] = 2;
        g_battle_work[2]++;
    }
}
