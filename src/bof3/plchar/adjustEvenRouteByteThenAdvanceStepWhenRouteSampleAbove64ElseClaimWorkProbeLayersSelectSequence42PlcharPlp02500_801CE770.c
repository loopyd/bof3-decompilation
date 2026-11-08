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

/* Step test of this payload, still an asm row of this target: it projects the
 * work position along the route byte and classifies the tile the step lands on,
 * answering one byte. The original calls it through a plain `jal` with nothing
 * set up in the argument registers, so it takes no arguments, and it masks the
 * answer to a byte before the branch.
 */
u8 func_801CEFF8(void);

/* Screen-space position routine of the shared EXE image, outside this payload:
 * it takes the two coordinate words and answers the signed halfword sample the
 * claim arm compares with the work reference halfword 0x3E. The exact
 * stepEvenRouteByteBackClaimWorkAndProbeRouteThreeAndFiveLayersThenSelectSequence42PlusHalfRouteAndAdvanceStepPlcharPlp02500_801CFB74
 * lift of this target declares the same word-returning prototype and narrows
 * the answer at the call site, which is what this body does too. */
s32 func_80154F28(s32 x, s32 y);

/* Route-parameterised position routine of the shared EXE image, outside this
 * payload: it takes the two coordinate words and a zero-extended route byte and
 * answers the sample the body narrows to a halfword and compares against 0x41. */
s32 func_80154F78(s32 arg0, s32 arg1, u8 arg2);

/* Work-sequence selector of the shared EXE image, outside this payload: it takes
 * a zero-extended byte sequence id and answers a word this body ignores. */
u32 func_8014D8D4(u8 arg0);

/* Shared main-executable cue request helper: this body passes the work halfword
 * 0x2C with 0x100 set. */
void func_8015DF18(u32 arg0);

/* @source 0x801CE770
 * @behavior Work-step handler of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload bytes, at
 *           word 0x801CFFB4, which is entry 3 of the sixteen-word handler run
 *           based at 0x801CFFA8, and the neighbouring entries of that run hold
 *           the other steps of the same work object, so no jal in this payload
 *           names it, it takes no arguments and it answers no value. Byte 0x08
 *           is the route index of the eight-entry route window, byte 0x02 the
 *           step counter, byte 0x0A the countdown the next handler decrements,
 *           byte 0x0B the step sign, byte 0x2B the claim byte, halfword 0x3E the
 *           reference sample and words 0x34 and 0x38 the work position. When the
 *           route byte is even the body first rewrites it: it stores the
 *           decremented route ((byte 8 - 1) & 7), runs the payload step test
 *           0x801CEFF8 and, only while that call answers zero, stores the
 *           neighbour two steps the other way ((byte 8 + 2) & 7) and runs the
 *           same test again; if that call answers zero too the byte is put back
 *           to (byte 8 - 2) & 7, which restores the value the first store left.
 *           It then projects the work position by adding the undoubled movement
 *           offsets of 0x80181B94/0x80181B98 selected by the route byte to the
 *           work words 0x34/0x38 and queries the route-parameterised position
 *           routine func_80154F78 for that point with the route byte. While the
 *           scratchpad gate byte 0x1F800000 reads nonzero and the narrowed
 *           answer is at least 0x41 the step stops at the route walk: it selects
 *           the shared work sequence ((byte 8 - 1) / 2 + 0x46) truncated to a
 *           byte and increments the step counter, which also happens again in
 *           the shared tail, so this arm advances the counter twice and never
 *           touches the countdown or the claim byte. Every other case claims the
 *           work first by storing 1 into byte 0x2B and then probes two further
 *           layer offsets, 0x80181BAC/0x80181BB0 with route 3 and
 *           0x80181BBC/0x80181BC0 with route 5: for each probe it again asks
 *           func_80154F78, and while the gate byte reads nonzero and that
 *           narrowed answer is at least 0x41 it asks the screen-space routine
 *           func_80154F28 for the layer point and clears the claim byte again
 *           when the signed reference halfword 0x3E is below that answer. It
 *           then requests the cue (work halfword 0x2C + 0x100) through
 *           func_8015DF18, selects the shared work sequence ((byte 8 - 1) / 2 +
 *           0x42) truncated to a byte, and arms byte 0x0A with 5. Both arms join
 *           on a shared tail that clears the step sign byte 0x0B and increments
 *           the step counter byte 0x02. The subtraction leaves both divisions
 *           signed, so route byte 0 selects sequence 0x42 or 0x46 and answers
 *           -1 halved, which is zero. The constant 1 that the two arms store
 *           into byte 0x2B is materialised in both branch delay slots, the
 *           increment of byte 0x02 of the route-walk arm issues in the delay
 *           slot of the jump to the tail, and the cue constant 5 is materialised
 *           after its call. The 0x20-byte frame keeps $ra at 0x18, the second
 *           probe coordinate at 0x14 and the first at 0x10 across the calls, and
 *           the pointer cell is loaded before the frame is created and re-read
 *           before every block and store. The 804-byte span at payload offset
 *           0x370 sits exactly between the two declared rows that bound it. The
 *           nearest exact sibling is
 *           adjustEvenRouteByteThenAdvanceStepWhenRouteSampleAbove64ElseClaimWorkProbeLayersSelectSequence42PlcharPlp27a00_801CEE74,
 *           whose 844-byte body repeats this statement order and differs in the
 *           route-walk arm alone: that body compared the screen-space answer
 *           less the reference halfword 0x3E with 0x41 and discarded the
 *           route-parameterised answer, while this body compares the
 *           route-parameterised answer itself, so that sibling supplied the
 *           shape and this span supplied its own comparison.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustEvenRouteByteThenAdvanceStepWhenRouteSampleAbove64ElseClaimWorkProbeLayersSelectSequence42PlcharPlp02500_801CE770(
    void) {
  s32 x;
  s32 y;
  s32 sample;

  if (!(D_1F800044[8] & 1)) {
    D_1F800044[8] = (D_1F800044[8] - 1) & 7;
    if (func_801CEFF8() == 0) {
      D_1F800044[8] = (D_1F800044[8] + 2) & 7;
      if (func_801CEFF8() == 0) {
        D_1F800044[8] = (D_1F800044[8] - 2) & 7;
      }
    }
  }
  x = *(s32*)(D_1F800044 + 0x34) + D_80181B94[D_1F800044[8] * 2];
  y = *(s32*)(D_1F800044 + 0x38) + D_80181B98[D_1F800044[8] * 2];
  sample = func_80154F78(x, y, D_1F800044[8]);
  if (D_1F800000[0] != 0 && (s16)sample >= 0x41) {
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
