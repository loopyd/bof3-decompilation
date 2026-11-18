#include "bof3/bof3.h"

/* Overlay-local pointer cell holding the shared 0x800E5148 work structure that
 * the sibling run member
 * seedSharedEffectObjectFromWorkPositionAndDistancePublishClearByte9ArmCountdownAdvanceWorkByte2ScenarioScena0700_801F7638
 * publishes into it. The cell sits in the
 * payload's final data span (the Splat textbin block labelled T_801FDE34) and
 * has no target map row; the generated undefined_syms_auto.txt row binds it. */
/* @source 0x801FDFBC @kind data */
extern u8* D_801FDFBC;

/* Quad helper at 0x801F97E0: reads three table pointers at +0x00, +0x04 and
 * +0x08 of its argument, picks one 0x20-byte-stride entry from each with
 * func_801783C8 / func_801782FC, fetches the texture page with GetTPage, and
 * submits a semi-transparent Gouraud quad at the shared primitive cursor with
 * SetDrawMode / SetPolyG4 / SetSemiTrans, walking the argument's unsigned
 * halfword at +0x10 (clamped to 0xFF) as its loop bound. Its result is a scaled
 * word the caller at 0x801F7638 stores; this caller discards it. */
s32 func_801F97E0(u8* obj);

/* @source 0x801F77AC
 * @behavior Per-frame tick of the effect object published at 0x801FDFBC: it
 * loads that pointer cell once (`lui $a0,%hi` / `lw $a0,%lo($a0)`, before its
 * frame is created), adds four to the object's unsigned halfword at +0x10 and
 * stores it back (the store is scheduled into the call's delay slot), then calls
 * the quad helper at 0x801F97E0 with that object as its only argument, so the
 * halfword this function advances is the loop bound the helper clamps to 0xFF
 * while it draws. Because the call clobbers the registers, the scratchpad
 * pointer slot at 0x1F800044 is read again afterwards (`lui $v1,0x1F80` /
 * `lw $v1,0x44($v1)`); the function then decrements that work object's signed
 * word at +0x0C (the store sits in the test's delay slot) and, when the
 * decremented countdown reaches zero, increments the object's unsigned byte at
 * +0x02 by one. Byte +0x02 is the handler index of this overlay's byte-2 state
 * machine, so that increment advances the run. Takes no arguments and returns
 * nothing; its 0x18-byte frame only keeps $ra and its single jr $ra epilogue
 * has the usual nop delay slot. The address is entry 2 of the four-word
 * in-image code-pointer run labelled D_801FDD8C (0x801F7638 / 0x801F76D0 /
 * this function / 0x801F7810), which dispatchWorkByte2HandlerScenarioScena0700_801F75F4
 * indexes with the work object's byte at +0x02. No in-image jal reaches the
 * address; that run's word at 0x801FDD90 is the only image reference to it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickObjectRampDrawQuadAndAdvanceHandlerScenarioScena0700_801F77AC(void) {
  u8* obj = D_801FDFBC;
  u8* work;

  *(u16*)(obj + 0x10) += 4;
  func_801F97E0(obj);
  work = SPAD_PTR_SLOT(u8, 0x44);
  if (--*(s32*)(work + 0x0C) == 0) {
    work[2]++;
  }
}
