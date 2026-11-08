#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-materialises the cell with its own `lui` before each probe block
 * and again before every claim store, so the source keeps the named-symbol form
 * rather than SPAD_PTR_SLOT, whose bare constant address is hoisted once into a
 * base register. The exact siblings of this overlay bind the cell the same way.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Scratchpad gate byte of the shared main executable: both probes of the body
 * compare their samples against the work reference halfword only while this
 * byte reads nonzero.
 * @source 0x1F800000 @kind data
 */
extern u8 D_1F800000[];

/* Layer probe offset words of the shared main-executable data section: one
 * first-coordinate and one second-coordinate offset per probed layer, added to
 * the work words 0x34 and 0x38. The body loads each of them as a whole word with
 * no index scaling, so they are scalars rather than table entries, and it
 * queries the 0x80181BAC/0x80181BB0 pair with route 3 and the
 * 0x80181BBC/0x80181BC0 pair with route 5.
 * @source 0x80181BAC @kind data
 */
extern s32 D_80181BAC;
/* @source 0x80181BB0 @kind data */
extern s32 D_80181BB0;
/* @source 0x80181BBC @kind data */
extern s32 D_80181BBC;
/* @source 0x80181BC0 @kind data */
extern s32 D_80181BC0;

/* Screen-space position routine of the shared EXE image, outside this payload:
 * it takes the two coordinate words and answers the signed halfword sample the
 * body compares with the work reference halfword 0x3E. The exact plp27a lift
 * declares the same word-returning prototype and narrows the answer at the call
 * site, which is what this body does too. */
s32 func_80154F28(s32 x, s32 y);

/* Route-parameterised position routine of the shared EXE image, outside this
 * payload: it takes the two coordinate words and a zero-extended route byte and
 * answers the sample the body compares against 0x41. The ui lift
 * isRouteSampleDifferenceAbove64 and the exact plp27a lift
 * adjustEvenRouteByteThenAdvanceStepWhenRouteSampleAbove64ElseClaimWorkProbeLayersSelectSequence42PlcharPlp27a00_801CEE74
 * declare the same prototype. */
s32 func_80154F78(s32 arg0, s32 arg1, u8 arg2);

/* @source 0x801D0868
 * @behavior Two-layer position refresh of the front-end work object read
 *           through the scratchpad pointer cell 0x1F800044, called at the end of
 *           the work-byte-3 steps 0x801CFBD8 and 0x801D0484 with nothing set up
 *           in the argument registers and with $v0 unread by either caller. It
 *           first claims work byte 0x2B by storing 1, then probes two layer
 *           points: it adds the layer offset words 0x80181BAC/0x80181BB0 to the
 *           work words 0x34/0x38 and asks the route-parameterised position
 *           routine func_80154F78 for that point with route 3, then repeats the
 *           probe with the pair 0x80181BBC/0x80181BC0 and route 5. For each
 *           probe, while the scratchpad gate byte 0x1F800000 reads nonzero and
 *           the narrowed sample is at least 0x41, it asks the screen-space
 *           routine func_80154F28 for the same point and clears work byte 0x2B
 *           when the signed reference halfword 0x3E is below that answer, so a
 *           probed layer that sits in front releases the claim. The body takes
 *           no arguments and returns nothing, and both probes are independent:
 *           the second one runs whether or not the first released the claim.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the 0x20-byte frame keeps $ra at 0x18, the
 * second coordinate at 0x14 and the first at 0x10, because both stay live across
 * two calls and each probe re-reads them for its position query. The first
 * pointer cell load, the constant 1 of the claim store and the route constant 3
 * are all issued before the frame's own spill stores, and the route constant 5
 * is materialised in the delay slot of both compare branches, which is the order
 * the exact plp27a lift at 0x801CEE74 compiles from the same statement block. The
 * reference halfword 0x3E is re-read through a fresh pointer load ahead of the
 * second call rather than kept in a register, so the source keeps the cell
 * expression at each access instead of hoisting a base.
 */
void claimWorkByte2BThenRefreshTwoLayerPositionsAndClearWhenLayerInFrontPlcharPlp02600_801D0868(
    void) {
  s32 x;
  s32 y;
  s32 sample;

  D_1F800044[0x2B] = 1;
  x = *(s32*)(D_1F800044 + 0x34) + D_80181BAC;
  y = *(s32*)(D_1F800044 + 0x38) + D_80181BB0;
  sample = func_80154F78(x, y, 3);
  if (D_1F800000[0] != 0 && (s16)sample >= 0x41) {
    if (*(s16*)(D_1F800044 + 0x3E) < (s16)func_80154F28(x, y)) {
      D_1F800044[0x2B] = 0;
    }
  }
  x = *(s32*)(D_1F800044 + 0x34) + D_80181BBC;
  y = *(s32*)(D_1F800044 + 0x38) + D_80181BC0;
  sample = func_80154F78(x, y, 5);
  if (D_1F800000[0] != 0 && (s16)sample >= 0x41) {
    if (*(s16*)(D_1F800044 + 0x3E) < (s16)func_80154F28(x, y)) {
      D_1F800044[0x2B] = 0;
    }
  }
}
