#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0, so no target-map row is
 * needed. The sibling lifts of this target read the same record through this
 * pointer: forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZero and
 * decrementWorkMotionWord44By1000ThenDecrementGlobalWorkByteBAndInvokeHelper
 * WhenTintChannel5FIsMinus80 decrement its global work byte 0xB exactly as this
 * body does, and decrementGlobalWorkByte9AndAdvanceWorkByte2WhenHelperReady
 * reads the record byte 9 this body counts down. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. This target's map binds no name there,
 * so the cell is reached through its address-anchored name, the spelling the
 * exact siblings of this target use. The original loads the pointer once ahead
 * of the frame setup and re-reads the cell after every call, so the cell stays
 * a plain named address and each post-call group re-materializes it. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Readiness poll helper of the shared main executable, bound by this target's
 * generated undefined_funcs_auto.txt row func_8014DAEC = 0x8014DAEC. The
 * original keeps only the low byte of the returned word as a guard
 * (andi $v0,0xFF then beqz), so the declared return type is that byte, the
 * prototype the exact sibling
 * decrementGlobalWorkByte9AndAdvanceWorkByte2WhenHelperReady measures with the
 * same call shape. */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */

/* Shared helper of the main executable, bound by this target's generated
 * undefined_funcs_auto.txt row func_800AFC98 = 0x800AFC98. Both call sites here
 * pass only the immediates 2 and 4 (addiu $a0,$zero,2 and addiu $a1,$zero,4)
 * and never read $v0 afterwards, so the original constrains neither argument
 * width nor the return type; word-width arguments keep the default promotion. */
void func_800AFC98(s32 arg0, s32 arg1);

/* @source 0x801EF58C
 * @behavior Work byte 2 step handler of this overlay work object, entry 1 of
 *           the four-entry work byte 2 handler table at 0x801EEC14 (0x801EF544,
 *           0x801EF58C, 0x801EF648, 0x801EF6AC) that the exact sibling
 *           dispatchWorkByte2HandlerThenInvokeD290WhenWorkByte0Nonzero copies
 *           into its own 0x28-byte frame at 0x10 and invokes with no arguments
 *           and no bounds test. No jal and no other payload word names this
 *           address; the only incoming word is the table entry at 0x801EEC18.
 *           It reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           before its frame exists and decrements that object's unsigned byte
 *           9 in place (lbu / addiu -1 / sb), testing the stored byte truncated
 *           to eight bits (andi 0xFF / bnez), so the zero arm runs only when
 *           that count reaches zero and the other arm runs while it stays
 *           non-zero.
 *           On zero it invokes the shared readiness helper func_8014DAEC and
 *           discards that answer, then invokes the shared helper func_800AFC98
 *           with the constant pair (2, 4). While the countdown is still
 *           non-zero it invokes func_8014DAEC again and tests the returned byte
 *           truncated to eight bits: a zero answer leaves the body, so only the
 *           frame epilogue runs, while a non-zero answer invokes
 *           func_800AFC98(2, 4), re-reads the pointer cell and stores 0 into
 *           work byte 9. Both arms that reach the shared tail decrement the
 *           shared battle work record's global work byte 0xB by one at byte
 *           width (lbu / addiu -1 / sb) through the pointer at 0x801EB4E0, then
 *           re-read the pointer cell once more and increment work byte 2 with a
 *           byte-width add, advancing the work byte 2 step index to entry 2 at
 *           0x801EF648. The 0x18-byte frame keeps only $ra at 0x10 across the
 *           calls, which is why the leading pointer load is scheduled before
 *           the frame setup. It takes no arguments, returns nothing and reads
 *           or writes no other state.
 *           Statement order, cell re-materialization grouping and declaration
 *           style follow the exact sibling
 *           decrementWorkByte9ThenInvokeHelperPairWhenZeroOrReadinessHelperReady
 *           ClearWorkByte9AndAdvanceWorkByte2 of emi/bmagic/magic008/00 at
 *           0x801F0A20, whose payload span measured word-identical to this span
 *           from the entry through the byte 9 clear and whose tail differs, so
 *           only this body's own tail store order comes from the bytes here.
 *           Roles of work byte 9, of record byte 0xB, of the two shared helpers
 *           and of the (2, 4) immediates are not established in this payload,
 *           so they stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenInvokeHelperPairWhenZeroOrReadinessHelperReadyClearWorkByte9ThenDecrementGlobalWorkByteBAndAdvanceWorkByte2BmagicMagic15403_801EF58C(
    void) {
  if (--D_1F800044[9] == 0) {
    func_8014DAEC();
    func_800AFC98(2, 4);
  } else if (func_8014DAEC() != 0) {
    func_800AFC98(2, 4);
    D_1F800044[9] = 0;
  } else {
    return;
  }
  D_801EB4E0[0xB]--;
  D_1F800044[2]++;
}
