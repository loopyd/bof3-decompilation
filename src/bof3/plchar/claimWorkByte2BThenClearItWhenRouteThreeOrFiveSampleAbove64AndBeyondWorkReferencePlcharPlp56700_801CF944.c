#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell before every block of the body and again before
 * each store, because a store through the pointer may alias the cell itself, so
 * the source keeps the named-symbol form and evaluates it again at every
 * access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Scratchpad gate byte of the shared main executable: the body runs a layer
 * query only while this byte reads nonzero.
 * @source 0x1F800000 @kind data
 */
extern u8 D_1F800000[];

/* Layer probe offset words of the shared main-executable data section: one
 * first-coordinate and one second-coordinate offset per probed layer, added to
 * the work words 0x34 and 0x38. The original loads each of them as a whole word
 * with no index scaling, so they are scalars rather than table entries.
 * @source 0x80181BAC @kind data
 */
extern s32 D_80181BAC;
/* @source 0x80181BB0 @kind data */
extern s32 D_80181BB0;
/* @source 0x80181BBC @kind data */
extern s32 D_80181BBC;
/* @source 0x80181BC0 @kind data */
extern s32 D_80181BC0;

/* Route-parameterised position routine of the shared EXE image, outside this
 * payload: it takes the two coordinate words and a zero-extended route byte and
 * answers the sample the body compares against 0x41. The ui lift
 * isRouteSampleDifferenceAbove64 and the plp27a lift
 * adjustEvenRouteByteThenAdvanceStepWhenRouteSampleAbove64ElseClaimWorkProbeLayersSelectSequence42PlcharPlp27a00_801CEE74
 * declare the same prototype. */
s32 func_80154F78(s32 arg0, s32 arg1, u8 arg2);

/* Screen-space position routine of the shared EXE image, outside this payload:
 * it takes the two coordinate words and answers the signed halfword sample the
 * body compares with the work reference halfword 0x3E. The plp27a lift above
 * declares the same prototype. */
s16 func_80154F28(s32 x, s32 y);

/* @source 0x801CF944
 * @behavior Layer refresh of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, run by the two work-step
 *           handlers 0x801CECB4 and 0x801CF560 of this payload before they
 *           select the shared work sequence. Byte 0x2B is the claim byte and
 *           the signed halfword 0x3E the work reference sample. The body first
 *           claims the work by storing 1 into byte 0x2B and then probes two
 *           fixed route offsets of the shared main-executable data section:
 *           0x80181BAC/0x80181BB0 with route 3 and 0x80181BBC/0x80181BC0 with
 *           route 5. For each probe it projects the work position by adding that
 *           offset pair to the work words 0x34 and 0x38, asks the
 *           route-parameterised position routine func_80154F78 for the sample of
 *           that point, and while the scratchpad gate byte 0x1F800000 reads
 *           nonzero and that sample narrowed to 16 bits is at least 0x41 it asks
 *           the position routine func_80154F28 for the same point and clears the
 *           claim byte when the reference halfword 0x3E is below that answer. A
 *           layer whose sample reads below the bound, or whose query is skipped
 *           by a zero gate byte, leaves the claim byte untouched, so the two
 *           probes only ever clear it. It takes no arguments, returns nothing,
 *           and the 0x20-byte frame only keeps $ra at 0x18 and the two projected
 *           coordinates at 0x14 and 0x10 across the calls. The route constants 3
 *           and 5 are materialised in the argument register per probe, and the
 *           5 of the second probe sits in the branch delay slots of the first
 *           block. The whole 356-byte span compared equal to the payload range
 *           at 0x801D0444 of the PLCHAR overlay plp016, whose own row was
 *           already exact when this seed was written, so a clone dry-run of that
 *           lift seeded this one; the same span also appears unchanged in the
 *           plp026, plp056, plp267, plp467 and plp678 overlays.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp56700_801CF944(
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
