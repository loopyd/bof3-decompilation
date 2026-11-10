#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F01B8
 * @behavior Increments the scratchpad work object's byte 9 modulo 256 and,
 * when the stored value equals 0x31, stores 5 into the shared battle work
 * record's byte 0xB, reloads the scratchpad work pointer and increments byte 2.
 * The pointer cells are at 0x1F800044 and 0x801EB4E0. Takes no arguments and
 * returns nothing; a payload table word at 0x801F0BCC names this entry.
 * The 96-byte span differs from emi/bmagic/magic060/00@0x801F031C only in
 * the compared immediate (0x31 here) and stored constant (5 here). This body
 * keeps that sibling's statement order, pointer local and unsigned byte value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9ThenSetGlobalWorkByteBTo5AndAdvanceWorkByte2At31BmagicMagic11303_801F01B8(void) {
    u8 *work;
    u8 value;

    work = g_battle_work;
    value = work[9] + 1;
    work[9] = value;
    if (value == 0x31) {
        D_801EB4E0[0xB] = 5;
        g_battle_work[2]++;
    }
}
