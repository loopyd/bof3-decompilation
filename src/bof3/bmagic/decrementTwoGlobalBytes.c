#include "bof3/bof3.h"

void clearWorkFive(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays,
 * bound at 0x1F800044 by this target's map. The body reads it once, before the
 * frame is built: the original materializes the cell (lui $v1,0x1F80 /
 * lw $v1,68($v1)) and drops the frame addiu and the $ra store into that load's
 * delay slot. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address through
 * the row D_801EB4E0 = 0x801EB4E0, so the name needs no target-map row, and the
 * record is touched one byte at a time. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF7A0
 * @behavior Subtracts two from the scratchpad work object byte at offset 0xA
 *           (lbu 0xA / addiu -2 / sb 0xA) and returns while the value it just
 *           stored, truncated to a byte (andi 0xFF of the stored register, no
 *           second load of the cell), stays nonzero. When that byte reached
 *           zero the body loads the shared battle work record pointer at
 *           0x801EB4E0 once, decrements that record's byte at offset 0xB by one
 *           at byte width (lbu / addiu -1 / sb) and calls this overlay's
 *           work-byte clear routine at 0x801EFDB0, the decrement store filling
 *           the call's delay slot. Takes no arguments, returns nothing and
 *           reads or writes no other state; the 0x18-byte frame only keeps $ra
 *           across that call. No jal in this payload reaches it: the fifth and
 *           last word of the pointer run at 0x801EFE00 holds 0x801EF7A0
 *           (0x801EF020, 0x801EF578, 0x801EF600, 0x801EF750, 0x801EF7A0), and
 *           dispatchWorkByte1HandlerBmagicMagic02103_801EF534 indexes that run
 *           with the work object's byte 1, so the table word fixes the entry
 *           boundary, not a missing caller. Same-target exact sibling
 *           add2ToWorkByte9AndAdvanceWorkByte2At10 (0x801EF750) supplied the
 *           cell binding and the byte test on the just-stored value, and the asm
 *           row of emi/bmagic/magic159/03 at 0x801EF014 carries the same
 *           subtract-two / decrement-global-byte-B walk with the cell load
 *           hoisted above the frame. Both spans differ from this one in their
 *           head and in their callee, so neither is byte-identical. The
 *           roles of work-object bytes 0xA and 0xB are not established, so both
 *           stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementTwoGlobalBytes(
    void) {
  if ((g_battle_work[0xA] -= 2) == 0) {
    D_801EB4E0[0xB]--;
    clearWorkFive();
  }
}
