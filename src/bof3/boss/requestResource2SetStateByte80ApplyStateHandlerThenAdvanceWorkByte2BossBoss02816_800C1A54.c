#include "bof3/bof3.h"

extern u8 *boss028CurrentWorkRecordState; /* @source 0x1F800044 @kind data */

/* Battle state byte in main RAM, outside this overlay's image; this target's
 * generated undefined_syms_auto.txt already binds the address, so no target-map
 * row is added here. The other boss lifts in this tree read the same cell as the
 * state selector they pass on to the shared per-state handler, and the boss and
 * bmagic lifts raise it to small constants (3, 4, 5 and 0xF); this entry writes
 * 0x80. The role of that particular value is not established by this body, so
 * the name reports the write and leaves the meaning offset-anchored. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Shared main-executable resource request helper, bound by this target's
 * generated undefined_funcs_auto.txt row. The exact same-target sibling
 * clearWorkByte24Byte2AAndMode48SetByte29To5ApplyLookup15CRequestResource6Copy
 * BattleRecordPositionSeedDistanceThenAdvanceStepBossBoss02816_800C2024
 * declares the same one-argument prototype and passes a resource id. */
extern void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 @kind function */

/* Shared main-executable state handler, bound by this target's generated
 * undefined_funcs_auto.txt row. It takes one byte tag, a work byte in the bmagic
 * lifts that call it and the battle state byte in the boss lifts, and is not
 * part of any payload here, so its body stays outside this lift. */
extern void func_800A9820(u8 state); /* @source 0x800A9820 @kind function */

/* @source 0x800C1A54
 * @behavior Entry 0 of this overlay's work-byte-2 code pointer run whose base
 *           word 0x800C2190 the exact sibling
 *           dispatchWorkByte2Table2190BossBoss02816_800C1A10 indexes with the
 *           work record byte 2 scaled by four; run word 0 at 0x800C2190 holds
 *           this address and no jal in the payload names this span, so the
 *           dispatcher reaches it through the run word and its parameter roles
 *           stay structural.
 *           It requests resource 2 through the shared main-executable helper
 *           func_8014D6B8, writes the constant 0x80 to the main-RAM battle state
 *           byte 0x80146384, runs the shared main-executable state handler
 *           func_800A9820 with that same 0x80, and finally takes the work record
 *           pointer published at the scratchpad cell 0x1F800044, increments that
 *           record's step byte 2 by one at byte width and stores it back, which
 *           hands the next frame of the run to entry 1 at 0x800C1AA0. The two
 *           0x80 constants are materialised separately by the original, one into
 *           the store's value register and one into the handler's argument.
 *           The 0x18-byte frame keeps $ra at 0x10 across the two direct calls,
 *           the body takes no arguments, and the value left in $v0 is the
 *           incremented step byte, which the dispatcher's indirect call discards.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResource2SetStateByte80ApplyStateHandlerThenAdvanceWorkByte2BossBoss02816_800C1A54(
    void) {
  func_8014D6B8(2);
  D_80146384 = 0x80;
  func_800A9820(0x80);
  boss028CurrentWorkRecordState[2]++;
}
