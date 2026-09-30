#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * by the battle lifts. The original re-materializes the cell address (lui
 * $vN,0x1F80 / lw $vN,68($vN)) for each of the two reads in this function, so
 * the cell is declared volatile like the exact sibling
 * initWorkMotionFieldsBmagicMagic00800_801F0330. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The sibling bmagic lifts (incrementWorkByte2WhenGlobalWorkByteBIs84
 * BmagicMagic11303_801EF8A4, setGlobalWorkByteBTo2ThenInvokeHelperWhenWorkByte
 * BZeroBmagicMagic15103_801EFDC0) read the same record's byte 0xB; splat binds
 * the address through this target's generated undefined_syms_auto.txt, so no
 * target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F04DC
 * @behavior Per-frame motion step of the write byte 0x2 state 4 handler of this
 * overlay: it is entry 4 of the six-entry handler table at 0x801EEC58
 * (0x801F0330, 0x801F037C, 0x801F03FC, 0x801F047C, 0x801F04DC, 0x801F0548),
 * which 0x801F0294 dispatches on work object byte 0x2. It reads the scratchpad
 * work-object pointer cell at 0x1F800044 once, loads that object's countdown
 * byte 0x9 (seeded with 0x1E by 0x801F0330) and the 16.16 motion pair words at
 * offsets 0x40 and 0x44 (seeded with 0x10000 by 0x801F0330), decrements the
 * countdown byte by one, subtracts 0x800 from the word at 0x40 and 0x1000 from
 * the word at 0x44, and writes the countdown byte back with sb and the two
 * advanced words back with sw, the second of which the original schedules into
 * the branch delay slot. When the new countdown byte is zero it additionally
 * stores 0xFF into byte 0xB of the shared battle work record at 0x801EB4E0,
 * re-reads the scratchpad pointer cell and increments the work object's state
 * byte 0x2 with a byte-width add (lbu / addiu 1 / sb), handing off to table
 * entry 5 (0x801F0548). The function is a leaf with no frame; it returns
 * nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementMotionCountdownAndSubtractMotionPairThenAdvanceWorkByte2BmagicMagic00800_801F04DC(void) {
  u8 *work;
  u8 counter;
  s32 word40;
  s32 word44;

  work = g_battle_work;
  counter = work[9] - 1;
  word40 = *(s32*)(work + 0x40) - 0x800;
  word44 = *(s32*)(work + 0x44) - 0x1000;
  work[9] = counter;
  *(s32*)(work + 0x40) = word40;
  *(s32*)(work + 0x44) = word44;
  if (counter == 0) {
    D_801EB4E0[0xB] = 0xFF;
    g_battle_work[2]++;
  }
}
