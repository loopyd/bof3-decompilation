#include "bof3/bof3.h"

/* Shared main-RAM front-end phase byte; this work step latches it to 2 just
 * before it advances the work object's dispatch byte. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Shared main-EXE front-end mode dispatcher, outside this payload. */
extern void func_80150224(s32 action_id); /* @source 0x80150224 */

/* @source 0x801F71DC
 * @behavior Work-object step handler of this overlay, the word at 0x801FBF00:
 * entry 3 of the twelve-word handler table D_801FBEF0 that the frame dispatcher
 * func_801F6EA4 indexes with byte 0x01 of the scratchpad work object published
 * at pointer slot 0x1F800044, and entry 0 of the sub-run D_801FBEFC that
 * func_801F7198 indexes with the same byte (both tables hold 0x801F71DC at those
 * slots). It takes no arguments and returns nothing. It first re-reads the work
 * pointer cell and zeroes the object's three colour-channel bytes at 0x5F, 0x5E
 * and 0x5D in that store order, the same tint triple the exact siblings
 * clearWorkTintArmCountdownAndAdvanceWorkByte1ScenarioScena1300_801F6EE8 (entry
 * 0) clears and bumpWorkRecordBytes5D5E5FOverCountdownThenAdvanceProgressAndWorkByte1ScenarioScena1300_801F7A1C
 * ramps by 8. It then re-reads the cell and stores 0x33 into the object's mode
 * byte 0x0B, re-reads the cell again and passes that byte, loaded unsigned, to
 * the shared front-end dispatcher func_80150224, latches the shared front-end
 * phase byte D_80143BB0 to 2, and finally re-reads the cell once more and moves
 * the object's dispatch byte 0x01 to 1, so the next frame runs entry 1
 * (0x801F7248), which waits on that same phase byte before it arms the object's
 * countdown. The 0x18-byte frame keeps only $ra at 0x10 across the one call, and
 * the frame setup fills the first cell load's delay slot. The cell is loaded four
 * times, once for the tint clear and once for each of the three later halves,
 * because the byte stores through the loaded pointer may alias it, which is also
 * why the argument is a fresh `lbu` of offset 0x0B rather than the 0x33 constant
 * just stored; the cell address is formed through the named `slot_offset` local
 * for the reason entry 0 records, since a constant-address cell would keep one
 * hoisted address across its reads.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTintQueueFrontEndMode33SetPhase2AndAdvanceWorkByte1ScenarioScena1300_801F71DC(void) {
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[0x5D] =
      PSX_REF(u8*, SPAD_BASE + slot_offset)[0x5E] =
          PSX_REF(u8*, SPAD_BASE + slot_offset)[0x5F] = 0;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[0xB] = 0x33;
  func_80150224(PSX_REF(u8*, SPAD_BASE + slot_offset)[0xB]);
  D_80143BB0 = 2;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1] = 1;
}
