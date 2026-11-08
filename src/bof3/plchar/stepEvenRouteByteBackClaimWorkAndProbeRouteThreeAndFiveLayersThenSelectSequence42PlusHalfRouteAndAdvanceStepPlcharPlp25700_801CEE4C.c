#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell before every block of the body and again before
 * each store, because a store through the pointer may alias the cell itself, so
 * the source keeps the named-symbol form and evaluates it again at every
 * access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Scratchpad gate byte of the shared main executable: the body runs every layer
 * position query of this step only while this byte reads nonzero.
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
 * payload: it takes the two coordinate words and a zero-extended route index
 * and answers the sample this body narrows to a halfword and compares against
 * 0x41. */
s32 func_80154F78(s32 arg0, s32 arg1, u8 arg2);

/* Screen-space position routine of the shared EXE image, outside this payload:
 * it takes the two coordinate words and answers the signed halfword sample the
 * body compares with the work reference halfword 0x3E, which this body narrows
 * at the call site. */
s32 func_80154F28(s32 x, s32 y);

/* Work-sequence selector of the shared EXE image, outside this payload: it takes
 * a zero-extended byte sequence id and answers a word this body ignores. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CEE4C
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload as entry 8 of the twenty-three-entry code pointer run that
 *           starts at 0x801CFEDC and whose base no word of this payload names:
 *           the body address appears once, as word 0x801CEE4C at 0x801CFEFC,
 *           and no jal in the payload targets it, so the overlay loader that
 *           steps the run calls it. Byte 0x08 is the route index of the
 *           eight-entry route window, byte 0x2B the claim byte, byte 0x02 the
 *           step counter, halfword 0x3E the reference sample and words 0x34 and
 *           0x38 the work position. While the route index is even the body
 *           first steps it one back inside the window by storing
 *           ((index - 1) & 7). It then claims the work by storing 1 into byte
 *           0x2B and probes two candidate layer positions: it adds the layer
 *           offsets 0x80181BAC and 0x80181BB0 to the work position and asks the
 *           route-parameterised position routine func_80154F78 for that point
 *           with route 3, then does the same with 0x80181BBC and 0x80181BC0 and
 *           route 5. For each probe, while the scratchpad gate byte 0x1F800000
 *           reads nonzero and the narrowed sample is at least 0x41, it asks the
 *           screen-space routine func_80154F28 for the same point and clears the
 *           claim byte again when the signed reference halfword 0x3E is below
 *           that answer; a layer whose sample reads below the bound, or whose
 *           query the gate byte skips, leaves the claim byte untouched, so the
 *           two probes only ever clear it. It finishes by selecting the shared
 *           work sequence 0x42 plus the route index halved, truncated to a byte,
 *           through func_8014D8D4, and incrementing the step counter byte 0x02.
 *           It takes no arguments, returns nothing and answers no value. The
 *           0x20-byte frame keeps $ra at 0x18, the second coordinate at 0x14 and
 *           the first at 0x10 across the calls, the pointer cell is loaded
 *           before the frame is created and re-read before every block and
 *           store, and the route constants 3 and 5 are materialised in the
 *           argument register per probe, the 5 of the second probe in the branch
 *           delay slots of the first probe. The halved route index shifts
 *           logically, because the byte is a zero-extended unsigned char, so
 *           route 0 selects sequence 0x42.
 *
 * Shape note: clone dry-run measured the 460-byte span byte-identical to the
 * exact plp025 lift at 0x801CFB74, payload offset 0x1774, and this lane also
 * measured the same span at 0x801CF750 of plp015, 0x801CF37C of plp056,
 * 0x801CE870 of plp457 and 0x801CE654 of plp567 and plp578; those payloads
 * shared this load address, and every absolute word the span held was a shared
 * main-executable address, so no word shifted. This source was seeded from the
 * plp025 lift and written against the bytes of this body, so the sibling supplied
 * statement order, operand forms and declaration style only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepEvenRouteByteBackClaimWorkAndProbeRouteThreeAndFiveLayersThenSelectSequence42PlusHalfRouteAndAdvanceStepPlcharPlp25700_801CEE4C(
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
