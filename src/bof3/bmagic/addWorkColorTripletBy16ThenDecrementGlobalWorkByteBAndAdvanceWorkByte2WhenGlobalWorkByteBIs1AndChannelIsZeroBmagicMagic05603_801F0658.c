#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the exact bmagic siblings
 * use (copyRecordByte27ThenAdvanceWorkByte2BmagicMagic05603_801EFFE8 and
 * decrementWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWorkByte2When
 * ChannelIs80BmagicMagic05603_801F00C8 read the same cell): the original reads
 * the cell again before each of the three channel stores and again before the
 * tint test, and the named symbol lets the assembler fold the constant address
 * into the load (lui $vN,0x1F80 / lw $vN,68($vN)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * bmagic siblings read the same record through this pointer; this entry tests
 * its byte 0xB before the colour group and decrements that byte once the
 * channel saturates. Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0658
 * @behavior Overlay step handler that finishes the colour fade the sibling
 *           decrementWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWork
 *           Byte2WhenChannelIs80BmagicMagic05603_801F00C8 starts. It first
 *           reads byte 0xB of the shared battle work record through 0x801EB4E0
 *           and runs only while that byte reads exactly 1. Inside that gate it
 *           adds 0x10 to the scratchpad work object's three colour channels at
 *           bytes 0x5D, 0x5E and 0x5F, reading the work-object pointer cell at
 *           0x1F800044 once per channel, and then reads the channel at 0x5D back
 *           through a signed byte view. Only when that red channel reads exactly
 *           zero does it decrement byte 0xB of the shared battle work record
 *           with a byte-width read-modify-write (lbu / addiu -1 / sb), re-read
 *           the work-object pointer cell and increment the object's handler byte
 *           2 with a byte-width add (lbu / addiu 1 / sb), advancing the
 *           dispatcher on work byte 2 to its next handler step. The cell is
 *           re-read before that final group because a store through the record
 *           pointer may alias it. Both branches reach the one return path, so
 *           the body has a single jr $ra. Leaf with no frame; takes no
 *           arguments and returns nothing, and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkColorTripletBy16ThenDecrementGlobalWorkByteBAndAdvanceWorkByte2WhenGlobalWorkByteBIs1AndChannelIsZeroBmagicMagic05603_801F0658(void) {
  if (D_801EB4E0[0xB] == 1) {
    g_battle_work[0x5D] += 0x10;
    g_battle_work[0x5E] += 0x10;
    g_battle_work[0x5F] += 0x10;

    if (((s8 *)g_battle_work)[0x5D] == 0) {
      D_801EB4E0[0xB]--;
      g_battle_work[2]++;
    }
  }
}
