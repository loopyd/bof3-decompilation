#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x8015477C @kind unknown
 * Shared distance helper over the word pair passed as (x, z); callers publish
 * its result scaled by 0x10000.
 */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* @source 0x801D0C48
 * @behavior Entry 0 of this overlay's step-handler run: the eight-word code
 * pointer table row angleStepHandlerTable labels this address as its first word, so the
 * dispatcher callAngleBandHandler reaches it
 * while the work object's unsigned byte at offset +0x1 is zero. It takes no
 * arguments and returns nothing. It seeds the work object's position pair with
 * 0x308000 at +0x34 and 0x740000 at +0x38, calls the shared distance helper
 * func_8015477C with the word just stored at +0x34 and the literal 0x740000,
 * and publishes that distance scaled by 0x10000 in the word at +0x3C. It then
 * seeds the second work word triple with 0x308000 at +0x0C, 0x718000 at +0x10
 * and 0xFEC00000 at +0x14, clears the angle halfword at +0x2E and the tint byte
 * at +0x5D, arms the countdown byte at +0x9 with 8 and advances the work byte
 * at +0x1, which hands the next frame to run entry 1. Its 0x18-byte frame keeps
 * $ra at 0x14 and $s0 at 0x10, and $s0 is what carries the 0x308000 seed across
 * the distance call so the same word fills both +0x34 and +0x0C.
 *
 * Evidence: the run word at 0x801D1F88 (payload offset 0x1F88) is 0x801D0C48
 * and the eight words from there are 0x801D0C48, 0x801D0CEC, 0x801D0DB0,
 * 0x801D0E5C, 0x801D0F44, 0x801D101C, 0x801D10C4 and 0x801D11B4, so this body
 * is element 0 of the run whose base the dispatcher loads. This address has no
 * in-payload `jal` and no other word of the payload holds it, so the run is its
 * only reference and the overlay loader outside the payload enters the
 * dispatcher, not this step. The field roles are the ones the neighbouring
 * steps of the same run read: raiseAngleBy16TintBy8DrawBandsOriginPrimitivesTickCountdownReload60QueueCue20DAndAdvanceWorkByte1ScenarioSce15ef000_801D0CEC
 * passes the blocks at +0x34 and +0x0C to the two origin-primitive drawing
 * calls, ramps the halfword at +0x2E and the tint byte at +0x5D, and ticks the
 * countdown byte at +0x9 that this body arms, so clearing +0x2E and +0x5D here
 * resets both ramps before the run draws anything.
 *
 * Shape evidence: the exact lift
 * seedWorkDistancePairArmCountdownAndAdvanceStepWorld02Area07714_801F4078
 * (0x801F4078 in emi/world02/area077/14) is the same statement order over the
 * same words - position pair into +0x34/+0x38, func_8015477C over the pair,
 * distance scaled by 0x10000 into +0x3C through a reloaded work pointer, then
 * the +0x0C seed and the +0x1 advance - and the exact lift
 * tickCountdownByte9SeedWorkWords34To3CAnd0CTo14AdvanceStateAndQueueCues202And203ScenarioScena0800_801F9AC0
 * spells the +0x0C/+0x10/+0x14 triple and the +0x3C distance word the same way.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateOriginTripleAngle(
    void) {
  u8* work;
  u8* next;
  s16 distance;

  work = D_1F800044;
  *(u32*)(work + 0x34) = 0x308000;
  *(u32*)(work + 0x38) = 0x740000;
  distance = func_8015477C(*(s32*)(work + 0x34), 0x740000);
  next = D_1F800044;
  *(u32*)(next + 0x3C) = distance << 16;
  *(u32*)(next + 0x0C) = 0x308000;
  *(u32*)(next + 0x10) = 0x718000;
  *(u32*)(next + 0x14) = 0xFEC00000;
  *(u16*)(next + 0x2E) = 0;
  *(u8*)(next + 0x5D) = 0;
  D_1F800044[9] = 8;
  D_1F800044[1]++;
}
