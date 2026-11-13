#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell the battle code publishes the current
 * work record into. Declared as a named cell so each load folds to lui plus an
 * offset load (lui $v1,0x1F80 / lw $v1,0x44($v1)), and because a store through
 * the loaded pointer may republish the cell the original re-reads it for the
 * step-index advance instead of keeping the pointer across the call. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and this target's exact lift of the step installer 0x800C1DCC already reads.
 * Its word at offset 0xE8 is the in-row pointer the installer copies from this
 * overlay's own word 0x800C3194. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Shared main-executable readiness helper at 0x801E30F8, enemyReadyOrHelper1 of
 * emi/battle/battle/03, declared with the unsigned byte return that target and
 * this overlay's exact step lifts record. This call site discards the answer,
 * which is why the original issues the jal with a nop in its delay slot and
 * never tests $v0. */
u8 func_801E30F8(void); /* @source 0x801E30F8 */

/* Shared main-executable effect submitter at 0x801E52F0, the same helper the
 * boss overlays' exact effect-submitting steps call with a halfword effect id.
 * This payload's only jal to it sits in this body, so the address is bound by
 * the generated undefined-funcs map. */
void func_801E52F0(u16 effect_id); /* @source 0x801E52F0 */

/* @source 0x800C1F0C
 * @behavior Work byte 9 handler of this overlay, reached from two of its own
 *           table runs rather than by a jal: word 0 of the run based at
 *           0x800C3274, which the byte-2 dispatcher 0x800C1EC8 indexes with
 *           work byte 2, and word 12 of the table based at 0x800C3244, which
 *           the byte-1 dispatcher 0x800C1D88 indexes with work byte 1. It
 *           first calls the shared readiness helper 0x801E30F8 with
 *           no arguments and discards the returned byte, then loads the
 *           work-object pointer published at the scratchpad cell 0x1F800044,
 *           decrements that object's unsigned byte 9 in place and tests the
 *           stored byte truncated to eight bits, so the guarded arm runs only
 *           when the countdown reaches zero. On zero it reads the current enemy
 *           work record pointer at 0x801EB4E8, takes that record's word 0xE8,
 *           the halfword-pair row 0x800C3194 whose halfword 0 reads 0x0602
 *           here, and submits that id through the shared submitter
 *           0x801E52F0; it then re-reads the scratchpad cell and advances work
 *           byte 2, the step index of the byte-2 run, by one. The 0x18-byte
 *           frame only keeps the return address at 0x10 across the calls, every
 *           delay slot in the span is a nop, and the body takes no arguments,
 *           returns no value and reads or writes no other state. The roles of
 *           work byte 9 as a countdown and of byte 2 as a step index follow the
 *           exact lifts of the sibling overlays. Statement order, the in-place
 *           countdown and the cell re-materialisation grouping follow the
 *           bmagic magic02003 exact lift at 0x801EF5A4, whose only differences
 *           are the shared helper's address and the effect submit this body's
 *           guarded arm adds before the advance.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callEnemyReadyThenCountDownWorkByte9AndSubmitEffectThenAdvanceWorkByte2BossBoss02513_800C1F0C(
    void) {
  u8 *work;
  u8 countdown;
  u8 *object;

  func_801E30F8();
  work = D_1F800044;
  countdown = work[9] - 1;
  work[9] = countdown;
  if (countdown == 0) {
    func_801E52F0(FIELD_REF(u16, FIELD_REF(u8 *, D_801EB4E8, 0xE8u), 0u));
    object = D_1F800044;
    object[2] += 1;
  }
}
