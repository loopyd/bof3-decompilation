#include "bof3/bof3.h"

/* Draw request of this payload at 0x801F5190, lifted in
 * claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190.c:
 * it takes two 16.16 points and queues the record pair that spans them. */
s32 claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190(
    s32 x0, s32 y0, s32 x1, s32 y1);

/* Stage drawer of this payload at 0x801F4DA4, still an asm boundary: it takes
 * the effect step index, indexes the shared 0x98-byte work records and the
 * payload stage tables with it, and answers 1 while the step stays under the
 * stage length and the stage transition answer afterwards. The original call
 * site copies its answer into the result register with a bare `addu`, so the
 * declared return here is 32-bit wide even though the body narrows itself. */
s32 func_801F4DA4(u32 arg0);

/* Shared cue request of the main executable: it takes the cue id in the low
 * half. */
void func_8015DF18(u16 arg0);

/* @source 0x801F4CF0
 * @behavior One step of the chained front-end effect driven by the shared step
 *           halfword at 0x801448EE. It queues the record pair that spans the
 *           fixed 16.16 point 0x428000/0x638000 and a second point on the same
 *           row whose x is 0x428000 minus the step times 0x10000 while the
 *           step stays under five and 0x3D8000 once it does not, so the pair
 *           grows one unit per step up to a fixed span of five units. At step
 *           five it also queues cue 0x206. It answers 1 while the step stays
 *           under five and, from step five on, the answer of the payload stage
 *           drawer func_801F4DA4 for steps under 0x38 and the constant 2 once
 *           the step reaches 0x38. Its one in-image caller is the front-end
 *           state machine at 0x801F45EC, which increments the shared halfword,
 *           stores the new value back and then separates the answers 0, 1 and
 *           the rest; the 0x20-byte frame keeps $ra, $s0 and $s1.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Name evidence: the queued pair is the same draw request the exact lifts
 * claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190
 * and drawStageExtentAndReturnNextStageCodeOnExpiryWorld03Area14514_801F504C
 * already document, and 0x428000/0x638000/0x3D8000 are 16.16 coordinates of the
 * same fixed window the other world overlays read as camera coordinates
 * (0x428000 appears as one in
 * setWorkState2To4WhenCameraOutsideFixedCoordinateWindowWorld03Area11913_801F3170).
 * The span this body queues is therefore 0x428000 minus the step for steps
 * under five and a fixed five units afterwards. The two answers it delegates
 * to are the stage walk of 0x801F4DA4, so the body is the front-end step that
 * clamps its own pair span and hands the rest of the sequence down; the step
 * bound 0x38 and the cue 0x206 stay literal.
 *
 * Shape notes from the original bytes: every use of the step masks `$a0` or
 * re-masks `$s0` in place, so the source spells the mask at each use and keeps
 * no masked local. The first arm materialises 0x428000, shifts the mask result
 * and subtracts, with the else arm reached by the `j` whose delay slot holds
 * the `subu`; the claim call then materialises all four arguments afresh. The
 * equal-to-five test is the listed `li $v0,5` / `bne` pair with the `sltiu` of
 * the following test reloaded in its delay slot, and both return arms compute
 * `andi $v0,$s1,0xFF` from the wider result local, which is why the result is
 * declared 32-bit here and the delegating callee is not narrowed to `u8`.
 */
u8 stepFrontEndPairSpanClampedToFiveThenDelegateStageWalkUntilStep56World03Area14514_801F4CF0(
    u32 arg0) {
  s32 result;
  s32 y;

  result = 1;
  if ((arg0 & 0xFFFFu) < 5u) {
    y = 0x428000 - (s32)((arg0 & 0xFFFFu) << 16);
  } else {
    y = 0x3D8000;
  }
  claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190(
      0x428000, 0x638000, y, 0x638000);
  if ((arg0 & 0xFFFFu) == 5u) {
    func_8015DF18(0x206);
  }
  if ((arg0 & 0xFFFFu) < 5u) {
    return result;
  }
  result = func_801F4DA4(arg0 & 0xFFFFu);
  if ((arg0 & 0xFFFFu) < 0x38u) {
    return result;
  }
  return 2;
}
