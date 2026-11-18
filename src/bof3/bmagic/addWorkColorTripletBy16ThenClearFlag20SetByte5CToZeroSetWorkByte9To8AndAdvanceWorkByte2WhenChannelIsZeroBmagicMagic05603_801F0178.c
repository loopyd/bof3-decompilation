#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the exact bmagic siblings
 * use (decrementWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWorkByte2
 * WhenChannelIs80BmagicMagic05603_801F00C8 and addWorkColorTripletBy16Then
 * DecrementGlobalWorkByteBAndAdvanceWorkByte2WhenGlobalWorkByteBIs1AndChannel
 * IsZeroBmagicMagic05603_801F0658 read the same cell): every statement that goes
 * through it must re-read the cell, because a store through the pointer it
 * supplied may alias the cell itself, and the original reloads it before each of
 * the three channel stores, before the saturation test and before the two tail
 * stores that follow the flag clear. The named symbol lets the assembler fold
 * the constant address into each load (lui $vN,0x1F80 / lw $vN,68($vN)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0178
 * @behavior Step 3 of the nine-word code pointer run based at 0x801EEC10,
 *           selected by work byte 2 of the scratchpad work object. It adds 0x10
 *           to the work-object colour channels at bytes 0x5D, 0x5E and 0x5F at
 *           byte width, reading the work-object pointer cell at 0x1F800044 once
 *           per channel and leaving no floor or ceiling test before those
 *           stores. It then re-reads the cell and the signed channel at 0x5D,
 *           and only when that red channel reads exactly zero does it clear bit
 *           0x20 of work byte 0 through the pointer the test loaded, re-read the
 *           cell and set work byte 0x5C to zero, re-read the cell and set the
 *           countdown byte 9 to 8, and re-read the cell once more to increment
 *           the handler byte at offset 2 at byte width, which advances the
 *           work-byte-2 dispatcher to step 4. No jal in this payload names
 *           0x801F0178, so it is reached through that run alone. Leaf with no
 *           arguments, no return value and no stack frame; reads and writes no
 *           other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkColorTripletBy16ThenClearFlag20SetByte5CToZeroSetWorkByte9To8AndAdvanceWorkByte2WhenChannelIsZeroBmagicMagic05603_801F0178(void) {
  u8 *work;

  g_battle_work[0x5D] += 0x10;
  g_battle_work[0x5E] += 0x10;
  g_battle_work[0x5F] += 0x10;

  work = g_battle_work;
  if (((s8 *)work)[0x5D] == 0) {
    work[0] &= 0xDF;
    g_battle_work[0x5C] = 0;
    g_battle_work[9] = 8;
    g_battle_work[2]++;
  }
}
