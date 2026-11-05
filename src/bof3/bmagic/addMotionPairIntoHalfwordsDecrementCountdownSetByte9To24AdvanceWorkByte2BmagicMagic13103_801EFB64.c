#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFB64
 * @behavior Per-frame step of the write byte 0x2 state 2 handler of this
 * overlay: it is the third of the eight entries at 0x801F1BD4, the handler
 * table that 0x801EFA68 dispatches on work object byte 0x2. Reads the
 * scratchpad work-object pointer cell at 0x1F800044 once and loads the
 * object's countdown byte 0x9, adds work words 0x18 and 0x1C into work words
 * 0xC and 0x10, then adds the re-read words 0xC and 0x10 into work halfwords
 * 0x2E and 0x30, writing the countdown byte back with sb and the two advanced
 * words with sw. The original schedules the 0x2E halfword store before the
 * branch and the 0x30 store into the branch delay slot. When the new
 * countdown byte is zero it stores 0x24 into byte 0x9, re-reads the
 * scratchpad pointer cell and increments state byte 0x2 with a byte-width add
 * (lbu / addiu 1 / sb), handing off to the next table entry (0x801EFBF8).
 * Leaf with no frame; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addMotionPairIntoHalfwordsDecrementCountdownSetByte9To24AdvanceWorkByte2BmagicMagic13103_801EFB64(void) {
    u8 *work;
    u32 *words;
    u16 *halves;
    u8 countdown;
    u8 *reset;
    u8 *step;

    work = g_battle_work;
    words = (u32 *)work;
    halves = (u16 *)work;
    countdown = work[9] - 1;
    words[3] += words[6];
    words[4] += words[7];
    halves[0x17] += words[3];
    halves[0x18] += words[4];
    work[9] = countdown;
    if (countdown == 0) {
        reset = g_battle_work;
        reset[9] = 0x24;
        step = g_battle_work;
        step[2]++;
    }
}
