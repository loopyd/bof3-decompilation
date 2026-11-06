#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is the named cell
 * symbol the exact sibling
 * incrementWorkByte2WhenWorkByteBZeroBmagicMagic15103_801F00B4 uses: the
 * symbolic address lets the assembler fold it into each load (lui $vN,0x1F80 /
 * lw $vN,68($vN)), so the original's two separate reads each re-materialise the
 * base. The plain cell is kept rather than the volatile one several siblings
 * use: the second read follows a store through the pointer the first read
 * loaded, and the sibling
 * seedMotionChannelTripletTintAndCountdownThenAdvanceWorkByte2
 * BmagicMagic15103_801EFE90 measured the volatile spelling identical on this
 * same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F00E8
 * @behavior Per-frame fade step of the overlay work object: it reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once, decrements
 *           the object's tint channel byte at offset 0x5D by one with a
 *           byte-width read-modify-write (lbu / addiu -1 / sb), sign-extends
 *           the byte it just stored (sll 24 / sra 24, no second load) and
 *           compares it against 8. Only when that signed channel equals 8 -
 *           the channel is the r0 tint byte the seeder 0x801EFE90 sets to 0x10
 *           and the sibling
 *           subtractWorkColorTripletBy3ThenAdvanceWorkByte2WhenChannelIs80
 *           BmagicMagic00800_801F11C4 reads back through a signed byte view -
 *           does it re-read the pointer cell and increment the state byte at
 *           offset 0x2 (lbu / addiu 1 / sb), advancing the overlay work byte 2
 *           handler sequence. The comparison constant is materialised with
 *           addiu $v0,$zero,8 and tested with bne. Leaf with no frame; returns
 *           nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkTintChannel5DThenIncrementWorkByte2WhenChannelIs8BmagicMagic15103_801F00E8(void) {
  u8 *work;
  u8 channel;

  work = g_battle_work;
  channel = work[0x5D] - 1;
  work[0x5D] = channel;
  if ((s8)channel == 8) {
    g_battle_work[2]++;
  }
}
