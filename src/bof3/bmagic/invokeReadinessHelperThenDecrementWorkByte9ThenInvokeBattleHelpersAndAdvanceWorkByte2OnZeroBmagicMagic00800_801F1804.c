#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, the same named cell this target's exact
 * lifts advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C,
 * copyRecordPositionPairOnFrameParityElseAddStepPairThenDecrementWorkByte9AndO
 * nZeroInvokeBattleHelpersWhenWorkByteBIsZeroAndAdvanceWorkByte2Bmagic
 * Magic00800_801F1090 and that body's twin at 0x801F1384 declare. The named
 * symbol lets the assembler fold the constant address into each load
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)), and the original re-reads the cell after
 * the helper calls because a store through the pointer may alias the cell
 * object. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte in main RAM, outside this overlay's image: the byte the
 * zero-countdown arm forwards to the helper at 0x800A9820. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, so no
 * target-map row is added here. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Shared readiness helper of the main executable at 0x8014DAEC, outside this
 * overlay's image; the exact sibling lift
 * setGlobalWorkByteBToFFAtWorkByte2State5ThenAdvanceWorkByte2WhenHelperReady
 * BmagicMagic00800_801F1400 declares the same prototype and tests the returned
 * byte. This call site issues the jal and never tests $v0, so the answer is
 * discarded. */
u8 func_8014DAEC(void);

/* Shared helper of the main executable at 0x800AFC98, outside this overlay's
 * image: invoked here with the constant pair (2, 0) and its result discarded,
 * so the original constrains neither argument width and word-width arguments
 * keep the default promotion. */
void func_800AFC98(s32 arg0, s32 arg1);

/* Shared helper of the main executable at 0x800A9820, outside this overlay's
 * image: this body forwards the shared battle byte 0x80146384 to it, the same
 * zero-extended byte argument the exact sibling lift
 * copyRecordPositionPairOnFrameParityElseAddStepPairThenDecrementWorkByte9AndO
 * nZeroInvokeBattleHelpersWhenWorkByteBIsZeroAndAdvanceWorkByte2Bmagic
 * Magic00800_801F1090 declares. */
void func_800A9820(u8 arg0);

/* @source 0x801F1804
 * @behavior Entry 1, entry 4 and entry 7 of the nine-entry handler run at
 *           0x801EECC8 (0x801F17BC, 0x801F1804, 0x801F1880, 0x801F18F4,
 *           0x801F1804, 0x801F1880, 0x801F18F4, 0x801F1804, 0x801F1880), which
 *           the dispatcher at 0x801F1708 copies into its own 0x40-byte frame
 *           and indexes with scratchpad work-object byte 2 before running the
 *           shared main-executable service func_8014D290 when work byte 0 is
 *           non-zero. No jal in this payload names this function, so those
 *           three table words are its only incoming paths. Per frame it calls
 *           the shared readiness helper func_8014DAEC and discards the returned
 *           byte, then re-materializes the work-object pointer cell at
 *           0x1F800044 and decrements that object's unsigned byte 9 in place
 *           (lbu / addiu -1 / sb), testing the stored byte truncated to eight
 *           bits (andi 0xFF / bnez), so the rest of the body runs only when that
 *           countdown reaches zero. On zero it invokes the shared helper
 *           func_800AFC98 with the constant pair (2, 0), the 2 filling that
 *           test's branch delay slot, then forwards the shared battle byte
 *           0x80146384 to the helper func_800A9820, and finally re-reads the
 *           pointer cell and increments work byte 2 with a byte-width add,
 *           moving dispatch to the next entry of the run. The 0x18-byte frame
 *           only keeps $ra at 0x10 across the calls; the function takes no
 *           arguments, returns nothing and reads or writes no other state.
 *           All 0x7C bytes of this span equal the span at 0x801F1384 (one cmp of
 *           the two payload offsets), so that sibling carries the same body and
 *           the same statements; its lift measured exact, and this source is
 *           written from its statement order, operand forms, declaration style
 *           and cell bindings. Statement order and cell re-materialization
 *           grouping follow the exact siblings of this target: 0x801F1090 for
 *           the (2, ...) helper pair followed by the forwarded battle byte in
 *           this order, and 0x801F1400 for the discarded readiness call and the
 *           trailing byte-2 increment. The cross-target lift of 0x801EF9E8 in
 *           emi/bmagic/magic004/03 carries the same countdown with a
 *           battle-kind mode select instead of this fixed pair and records
 *           itself exact; it is shape evidence only. Roles of work byte 9, of
 *           the three shared helpers and of the mode immediate are not
 *           established here, so they stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeReadinessHelperThenDecrementWorkByte9ThenInvokeBattleHelpersAndAdvanceWorkByte2OnZeroBmagicMagic00800_801F1804(void) {
  func_8014DAEC();
  if (--g_battle_work[9] == 0) {
    func_800AFC98(2, 0);
    func_800A9820(D_80146384);
    g_battle_work[2]++;
  }
}
