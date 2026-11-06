#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * by the battle lifts. The original re-materializes the cell address (lui
 * $vN,0x1F80 / lw $vN,68($vN)) for each of the two accesses after the helper
 * call, so the cell is declared volatile like the exact siblings
 * initWorkMotionFieldsBmagicMagic00800_801F0330 and the motion step at
 * 0x801F04DC. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Original callee returns a table-selected byte and consumes no arguments. */
u8 func_800AFF98(void);

/* @source 0x801F037C
 * @behavior Per-frame motion step of work byte 0x2 state 1 of this overlay: it
 * is entry 1 (index 1) of the six-entry handler table at 0x801EEC58 (0x801F0330,
 * 0x801F037C, 0x801F03FC, 0x801F047C, 0x801F04DC, 0x801F0548), which 0x801F0294
 * copies into its own frame and dispatches on the scratchpad work object byte
 * 0x2, the state 0x801F0330 advances from 0 to 1. It reads the work-object
 * pointer cell at 0x1F800044 once, loads that object's countdown byte 0x9
 * (seeded with 0x1E by 0x801F0330) and the 16.16 motion pair words at offsets
 * 0x40 and 0x44 (both seeded with 0x10000 by 0x801F0330), decrements the
 * countdown byte by one, adds 0x400 to the word at 0x40 and 0x800 to the word at
 * 0x44, and writes the countdown byte back with sb and the two advanced words
 * back with sw, the second of which the original schedules into the branch delay
 * slot. When the new countdown byte is zero it calls the shared helper at
 * 0x800AFF98 with no arguments, stores the returned byte in work byte 0x9, then
 * re-reads the scratchpad pointer cell and increments work byte 0x2 with a
 * byte-width add (lbu / addiu 1 / sb), handing off to table entry 2
 * (0x801F03FC). The 0x18-byte frame only keeps $ra across that call; the
 * function returns nothing and reads or writes no other state. It mirrors the
 * exact sibling entry 4 at 0x801F04DC, which subtracts from the motion pair and
 * stores 0xFF into byte 0xB of the shared battle work record instead of
 * re-seeding work byte 0x9 from the helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementMotionCountdownAndAdvanceMotionPairThenSeedWorkByte9FromHelperAFF98AndAdvanceWorkByte2BmagicMagic00800_801F037C(void) {
  u8 *work;
  u8 counter;
  s32 word40;
  s32 word44;

  work = g_battle_work;
  counter = work[9] - 1;
  word40 = *(s32*)(work + 0x40) + 0x400;
  word44 = *(s32*)(work + 0x44) + 0x800;
  work[9] = counter;
  *(s32*)(work + 0x40) = word40;
  *(s32*)(work + 0x44) = word44;
  if (counter == 0) {
    g_battle_work[9] = func_800AFF98();
    g_battle_work[2]++;
  }
}
