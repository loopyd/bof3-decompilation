#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original keeps the value of the first load across the route rewrite, because
 * that read and its store are one basic block, and then re-reads the cell before
 * every later block and store, because a store through the pointer may alias the
 * cell itself, so the source keeps the named-symbol form and evaluates it again
 * at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Scratchpad gate byte of the shared main executable: the body runs each layer
 * position query of the claim probes only while this byte reads nonzero.
 * @source 0x1F800000 @kind data
 */
extern u8 D_1F800000[];

/* Layer probe offset words of the shared main-executable data section: one
 * first-coordinate and one second-coordinate offset per probed layer, added to
 * the work words 0x34 and 0x38. The original loads each of them as a whole word
 * with no index scaling, so they are scalars rather than table entries, and this
 * target's generated undefined_syms_auto.txt binds all four addresses.
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
 * body compares with the work reference halfword 0x3E. The world00/area17213 and
 * plp27a lifts declare the same word-returning prototype and narrow the answer
 * at the call site, which is what this body does too. This target's generated
 * undefined_funcs_auto.txt binds the address. */
s32 func_80154F28(s32 arg0, s32 arg1);

/* Route-parameterised position routine of the shared EXE image, outside this
 * payload: it takes the two coordinate words and a zero-extended route id and
 * answers the sample the body compares against 0x41. The ui lift
 * isRouteSampleDifferenceAbove64 declares the same prototype, and this target's
 * generated undefined_funcs_auto.txt binds the address. */
s32 func_80154F78(s32 arg0, s32 arg1, u8 arg2);

/* Work-sequence selector of the shared EXE image, outside this payload: it takes
 * a zero-extended byte sequence id and answers a word this body ignores. The
 * sibling selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp01500_801CF610
 * of this target declares the same prototype. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF750
 * @behavior Overlay step handler of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. The body address appears once in
 *           this payload, as word 0x801CFBBC, entry 14 of the sixteen-word
 *           code-pointer run that starts at 0x801CFB84, and no jal in the
 *           payload names it, so the overlay loader that steps that run is what
 *           calls it. Byte 0x08 is the route index of the eight-entry route
 *           window, byte 0x02 the step counter, byte 0x2B the claim byte and the
 *           halfword 0x3E the reference sample each layer position query is
 *           compared with. When the route byte is even the body first rewrites
 *           it, storing the decremented route ((byte 8 - 1) & 7) through the
 *           same pointer value that the test read, and it then claims the work
 *           by storing 1 into byte 0x2B. Two layer probes follow. Each one adds a
 *           fixed layer offset pair, 0x80181BAC/0x80181BB0 for route id 3 and
 *           0x80181BBC/0x80181BC0 for route id 5, to the work position words
 *           0x34/0x38, asks the shared route-parameterised routine func_80154F78
 *           for the sample of that point and, while the scratchpad gate byte
 *           0x1F800000 reads nonzero and that sample narrowed to a halfword is at
 *           least 0x41, asks the shared position routine func_80154F28 for the
 *           layer point and clears the claim byte again when the signed reference
 *           halfword 0x3E is below that answer. The body then selects the shared
 *           work sequence through func_8014D8D4 with the halved route index
 *           offset by 0x42, truncated to a byte, and increments the step counter
 *           byte 0x02. It takes no arguments and returns nothing. The 0x20-byte
 *           frame keeps $ra at 0x18, the second coordinate at 0x14 and the first
 *           at 0x10 across the calls, and the pointer cell is loaded before the
 *           frame is created. The constant 1 that the two arms store into byte
 *           0x2B is materialised in both incoming delay slots of the claim store,
 *           the sequence offset 0x42 is added in the delay slot of the selector
 *           call, and the halving is a logical shift because the route byte is
 *           unsigned, so the sequence id provably fits a byte and no mask is
 *           emitted after the addition.
 *
 * Shape note: the 0x148-byte span 0x801CF78C..0x801CF8D3 of this body, which
 * holds the claim store and both layer probes, compared byte for byte against
 * 0x801CEFE8 of the plp27a overlay claim arm at 0x801CEE74 while that lift was
 * already exact, and 0 of those 328 bytes differed. That copy spans a different
 * target with different selector and callee addresses plus a route-walk arm this
 * body does not carry, so it is shape evidence and not clone eligibility.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustEvenRouteByteThenClaimWorkProbeTwoLayerOffsetsSelectSequence42PlusHalvedRouteAndAdvanceStepPlcharPlp01500_801CF750(
    void) {
  s32 x;
  s32 y;
  s32 sample;

  if (!(D_1F800044[8] & 1)) {
    D_1F800044[8] = (D_1F800044[8] - 1) & 7;
  }
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
  func_8014D8D4((u8)(D_1F800044[8] / 2 + 0x42));
  D_1F800044[2] = D_1F800044[2] + 1;
}
