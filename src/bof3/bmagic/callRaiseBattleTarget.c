#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, outside this overlay's
 * image. This target's map binds no name there, so the cell is reached through
 * its address-anchored name, the spelling the exact siblings
 * decrementGlobalWorkByte9AndAdvanceWorkByte2WhenHelperReady
 * BmagicMagic15403_801EF648 and decrementWorkByte9ThenSeedWorkHeaderAndAdvance
 * WorkByte2BmagicMagic15403_801EF1EC bind the same cell with. The original
 * re-materializes the cell address for the byte 1 access after the calls,
 * because a callee may repoint it, so the body reads the cell twice. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0, so no target-map row is
 * needed. The original reads record byte 0, masks it with 0xBF and stores it
 * back through one register that holds the record base. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* Shared battle target byte in main RAM, outside this overlay's image. This
 * target's generated undefined_syms_auto.txt binds the address and the
 * original loads it at byte width (lbu) and passes it zero-extended, so the
 * declared width is a byte. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Shared battle event dispatcher of the main executable at 0x800AEF98. This
 * target's generated undefined_funcs_auto.txt binds the address; the original
 * passes an event selector in the first argument and a value in the second and
 * uses no result, which is the prototype the exact sibling lift
 * callAFC54ThenCallAEF98With4And0ClearRecordFlag40AndAdvanceWorkByte1
 * WhenWorkByteBZeroBmagicMagic01500_801EF594 declares for the same call. */
extern void func_800AEF98(s32 event, s32 value); /* @source 0x800AEF98 @kind function */

/* Shared main-executable helper that raises battler status flags: its exact
 * callers pass the shared battle target byte with one flag word and it raises
 * that flag on the battlers that byte selects. This target's generated
 * undefined_funcs_auto.txt binds the address. */
extern void func_800A9610(u8 target, s32 flags); /* @source 0x800A9610 @kind function */

/* @source 0x801EEFE0
 * @behavior Entry 2 of the four-entry work byte 1 handler run at 0x801EEC04
 *           (0x801EEC90, 0x801EEE94, 0x801EEFE0, 0x801EF060), a run that
 *           occupies the overlay's own table words and that the exact sibling
 *           dispatcher dispatchWorkByte1HandlerBmagicMagic15403_801EEC24 copies
 *           into its frame and invokes with no arguments, so the payload holds
 *           no jal to this address: an overlay dispatch entry, not a called
 *           function. It reads the scratchpad work-object pointer cell at
 *           0x1F800044 once before its frame exists and tests the work object's
 *           unsigned byte 9 (lbu / bnez). While that byte is nonzero the
 *           handler returns without writing anything. At zero it dispatches
 *           event 4 with the value 0 through the shared main-executable
 *           dispatcher func_800AEF98 (addiu $a0,0x4 / addu $a1,$zero,$zero),
 *           clears bit 0x40 of byte 0 of the shared battle work record at
 *           0x801EB4E0 (lbu / andi 0xBF / sb through one register that holds
 *           the record base), raises flag 0x10 on the battle target byte at
 *           0x80146384 through the shared main-executable helper func_800A9610
 *           (lbu $a0 / addiu $a1,0x10), and then re-reads the work-object
 *           pointer cell and increments the work object's unsigned byte 1 by
 *           one, advancing the work byte 1 state to run entry 3 (0x801EF060).
 *           The 0x18-byte frame keeps only $ra at 0x10; the handler takes no
 *           arguments, returns nothing and reads or writes no other state.
 *           Statement order, operand forms, offset-local spelling and
 *           declaration style follow the exact lift
 *           callAFC54ThenCallAEF98With4And0ClearRecordFlag40AndAdvanceWorkByte1
 *           WhenWorkByteBZeroBmagicMagic01500_801EF594, whose 124-byte body is
 *           this span's neighbour with the same guard, clear and advance tail;
 *           this span's own bytes supplied the guarded work byte (9), the
 *           record mask 0xBF, and the helper func_800A9610 call site with
 *           0x80146384 and flag 0x10 in place of that sibling's buffer-copy
 *           call. A payload scan of all 398 extracted binaries found no
 *           byte-identical copy of this 128-byte span, masked on its call
 *           targets, so the name and the body come from this span alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callRaiseBattleTarget(
    void) {
  if (D_1F800044[9] == 0) {
    func_800AEF98(4, 0);
    D_801EB4E0[0] &= 0xBF;
    func_800A9610(D_80146384, 0x10);
    D_1F800044[1]++;
  }
}
