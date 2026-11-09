#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell before every block of the body and again before
 * each store, because a store through the pointer may alias the cell itself, so
 * the source keeps the named-symbol form and evaluates it again at every
 * access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Scratchpad gate byte of the shared main executable: the body runs every
 * position query of this step only while this byte reads nonzero.
 * @source 0x1F800000 @kind data
 */
extern u8 D_1F800000[];

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the active work area's
 * route byte at offset 0x08. The original scales that byte by eight once, which
 * is element index route * 2, and adds each offset undoubled, so both bases stay
 * their own symbol.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table
 */
extern s32 D_80181B98[];

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

/* Step test of this payload, exact in this target: it projects the work
 * position along the route byte and classifies the tile the step lands on,
 * returning one byte. The original calls it through a plain `jal` with nothing
 * set up in the argument registers, so it takes no arguments, and it masks the
 * answer to a byte before the branch. The two jal words at 0x801CECA8 and
 * 0x801CECD8 of this span are the only sites that reach it in this payload.
 */
u8 testProjectedWorkPositionAgainstWorkRecordsOrMapCodeF2PlcharPlp57800_801CF528(
    void);

/* Screen-space position routine of the shared EXE image, outside this payload:
 * it takes the two coordinate words and answers the signed halfword sample the
 * body compares with the work reference halfword 0x3E. The world00/area17213
 * lift declares the same word-returning prototype and narrows the answer at the
 * call site, which is what this body does too. */
s32 func_80154F28(s32 x, s32 y);

/* Route-parameterised position routine of the shared EXE image, outside this
 * payload: it takes the two coordinate words and a zero-extended route byte and
 * answers the sample the body compares against 0x41. The ui lift
 * isRouteSampleDifferenceAbove64 declares the same prototype. */
s32 func_80154F78(s32 arg0, s32 arg1, u8 arg2);

/* Work-sequence selector of the shared EXE image, outside this payload: it takes
 * a zero-extended byte sequence id and answers a word this body ignores. The
 * declared siblings selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp57800_801CEB38
 * and selectArrivalSequenceForWorkByte8AndClearWorkByte7WhenHelperReadyElseClearWorkByte2BAndState12BWhenFrontEndRowUnoccupiedPlcharPlp57800_801CE92C
 * of this target declare the same prototype. */
u32 func_8014D8D4(u8 arg0);

/* Shared main-executable cue request helper; the bmagic and ui lifts declare the
 * same single-argument prototype and pass a cue id, and this body passes the
 * work halfword 0x2C with 0x100 set. */
void func_8015DF18(u32 arg0);

/* @source 0x801CEC78
 * @behavior Work-step handler of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. Its only in-image reference is
 *           the word 0x801CEC78 at 0x801CFADC, entry 9 of this payload's
 *           twenty-three-word handler run based at 0x801CFAB8, and the byte-2
 *           dispatcher of this payload func_801CEC34 indexes that run from the
 *           interior base 0x801CFADC without bounding the index, so the loader
 *           that steps the run and that dispatcher are what enter this body; no
 *           jal of the payload names it. Byte 0x08 is the route index of the
 *           eight-entry route window, byte 0x02 the step counter, byte 0x0A the
 *           countdown the next handler decrements, byte 0x0B the step sign, byte
 *           0x2B the claim byte and the halfword 0x3E the reference sample the
 *           shared position routine is compared with. When the route byte is
 *           even the body first rewrites it: it stores the decremented route
 *           ((byte 8 - 1) & 7), runs the payload step test 0x801CF528 and, only
 *           while that call reads zero, stores the neighbour two steps the other
 *           way ((byte 8 + 2) & 7) and runs the same test again; if that call
 *           reads zero too the byte is put back to (byte 8 - 2) & 7, which
 *           leaves it unchanged. It then projects the work position by adding
 *           the undoubled movement offsets of 0x80181B94/0x80181B98 selected by
 *           the route byte to the work words 0x34/0x38, asks the shared position
 *           routine func_80154F28 for the sample of that point, keeps that
 *           sample less the unsigned reference halfword 0x3E, and queries the
 *           route-parameterised routine func_80154F78 for the same point with
 *           the route byte. While the scratchpad gate byte 0x1F800000 reads
 *           nonzero and the narrowed difference is at least 0x41 the step stops
 *           at the route walk: it selects the shared work sequence ((byte 8 - 1)
 *           / 2 + 0x46), truncated to a byte, and increments the step counter,
 *           which also happens again in the shared tail, so this arm advances
 *           the counter twice and never touches the countdown or the claim byte.
 *           Every other case claims the work first by storing 1 into byte 0x2B
 *           and then probes two further layer offsets, 0x80181BAC/0x80181BB0
 *           with route 3 and 0x80181BBC/0x80181BC0 with route 5: for each probe
 *           it again asks func_80154F78, and while the gate byte reads nonzero
 *           and that sample is at least 0x41 it asks func_80154F28 for the layer
 *           point and clears the claim byte again when the signed reference
 *           halfword 0x3E is below that answer. It then requests the cue (work
 *           halfword 0x2C + 0x100) through func_8015DF18, selects the shared
 *           work sequence ((byte 8 - 1) / 2 + 0x42) truncated to a byte, and
 *           arms byte 0x0A with 5. Both arms join on a shared tail that clears
 *           the step sign byte 0x0B and increments the step counter byte 0x02.
 *           The subtraction leaves both divisions signed, so route byte 0 ==
 *           sequence 0x42/0x46. It takes no arguments and returns nothing. The
 *           0x20-byte frame keeps $ra at 0x18, the second coordinate at 0x14 and
 *           the first at 0x10 across the calls; the pointer cell is loaded
 *           before the frame is created and re-read before every block and
 *           store. The constant 1 that the two arms store into byte 0x2B is
 *           materialised in both branch delay slots, the increment of byte 0x02
 *           of the route-walk arm issues in the delay slot of the jump to the
 *           tail, and the cue constant 5 is materialised after its call. The
 *           844-byte span compares word for word with the plp27a span at
 *           0x801CEE74: exactly three words differ, the two jal words to that
 *           target own step test at 0x801CECA8 and 0x801CECD8 and the jump word
 *           to the shared tail at 0x801CEDE0, and each differs by 0x7F, which is
 *           that span offset delta 0x1FC scaled to a word index, so this body is
 *           the shared handler written against this payload own addresses. The
 *           plp457 span at 0x801CEE94 shows the same three words shifted by its
 *           own delta.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustEvenRouteByteThenAdvanceStepWhenRouteSampleAbove64ElseClaimWorkProbeLayersSelectSequence42PlcharPlp57800_801CEC78(
    void) {
  s32 x;
  s32 y;
  s32 sample;
  s16 delta;

  if (!(D_1F800044[8] & 1)) {
    D_1F800044[8] = (D_1F800044[8] - 1) & 7;
    if (testProjectedWorkPositionAgainstWorkRecordsOrMapCodeF2PlcharPlp57800_801CF528() ==
        0) {
      D_1F800044[8] = (D_1F800044[8] + 2) & 7;
      if (testProjectedWorkPositionAgainstWorkRecordsOrMapCodeF2PlcharPlp57800_801CF528() ==
          0) {
        D_1F800044[8] = (D_1F800044[8] - 2) & 7;
      }
    }
  }
  x = *(s32*)(D_1F800044 + 0x34) + D_80181B94[D_1F800044[8] * 2];
  y = *(s32*)(D_1F800044 + 0x38) + D_80181B98[D_1F800044[8] * 2];
  delta = func_80154F28(x, y) - *(u16*)(D_1F800044 + 0x3E);
  func_80154F78(x, y, D_1F800044[8]);
  if (D_1F800000[0] != 0 && delta >= 0x41) {
    func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x46));
    D_1F800044[2] = D_1F800044[2] + 1;
  } else {
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
    func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
    func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
    D_1F800044[0xA] = 5;
  }
  D_1F800044[0xB] = 0;
  D_1F800044[2] = D_1F800044[2] + 1;
}
