#include "bof3/bof3.h"

void func_801F34B8(void);
u32 func_801F3034(void);

/* @source 0x801F2CC8
 * @behavior Overlay step handler, entry 1 of the local state-byte-1 handler
 *           table at 0x801F56E8: runs the 0x37-entry motion integrator at
 *           0x801F34B8, then walks the 16 records of the 0x28-byte-stride work
 *           table at 0x800E4800 through the handler table at 0x801F56F4
 *           (0x801F3034, which leaves 1 in its return register once at least
 *           one record ran a handler and 0 when none did). Only when the low
 *           byte of that result is zero does it advance the dispatch byte at
 *           +1 of the work record published at scratchpad pointer cell
 *           0x1F800044, handing the step over to the next handler of the same
 *           table. Takes no arguments and returns nothing; the roles of both
 *           callees and of the record fields stay anchored to their addresses.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The frame slot below the work pointer is a reconstruction of a declaration
 * the original translation unit carried but this overlay's emitted code never
 * references: the original frame is 0x28 while this function's own locals need
 * none, it saves only $ra at 0x20, and no instruction in the original body
 * reads or writes the 16 bytes at 0x10-0x1F (the only $sp offsets are 0x20).
 * Declaring one 16-byte local restores that reservation exactly; the name and
 * type are a reconstruction choice constrained only by the slot size. Same
 * recorded shape as the frame slots in func_801F7670 and
 * seedWorkDistancePairArmCountdownAndAdvanceStepWorld02Area07714_801F4078.
 * Flagged for independent review; removing the declaration leaves only the
 * frame-size/offset residual (16/20 instructions, 80 bytes, first difference at
 * 0x801F2CC8).
 */
void advanceWorkByte1WhenNoRecordActiveWorld02Area10814_801F2CC8(void) {
  u32 unusedFrameSlot[4];

  (void)unusedFrameSlot;
  func_801F34B8();
  if ((func_801F3034() & 0xFF) == 0) {
    SPAD_PTR_SLOT(u8, 0x44)[1]++;
  }
}
