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
 * with no index scaling, so they are scalars rather than table entries. The
 * target-local lift at 0x801D066C declares the same four words.
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
 * it takes the two coordinate words and answers the sample the body narrows to
 * a halfword and compares with the work reference halfword 0x3E. The body
 * narrows the answer at the call site, so the prototype stays word-returning. */
s32 func_80154F28(s32 x, s32 y);

/* Route-parameterised position routine of the shared EXE image, outside this
 * payload: it takes the two coordinate words and a zero-extended route index
 * and answers the sample this body narrows to a halfword and compares against
 * 0x41. This target's generated undefined_funcs_auto.txt binds 0x80154F78 and
 * the target-local lift at 0x801D066C declares the same prototype. */
s32 func_80154F78(s32 arg0, s32 arg1, u8 arg2);

/* Work-sequence selector of the shared EXE image, outside this payload: it takes
 * a zero-extended byte sequence id and answers a word this body ignores. This
 * target's generated undefined_funcs_auto.txt binds 0x8014D8D4, and the sibling
 * lift at 0x801CF23C of this payload declares the same prototype. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF37C
 * @behavior Entry 9 of the front-end work handler run based at 0x801D0858. Its
 *           only in-image reference is the word 0x801CF37C at 0x801D087C, index
 *           0 from the base 0x801D087C that the step dispatcher at 0x801CF338
 *           addresses after scaling work byte 2 by four, so a work byte 2 of 0
 *           reaches this body; no jal in the payload targets it. Work object of
 *           the scratchpad pointer cell 0x1F800044: byte 0x08 is the route index
 *           of the eight-entry route window, byte 0x2B the claim byte, byte 0x02
 *           the step counter, signed halfword 0x3E the reference sample and
 *           words 0x34 and 0x38 the work position. While the route index is even
 *           the body first steps it one back inside the window by storing
 *           ((index - 1) & 7). It then claims the work by storing 1 into byte
 *           0x2B and probes two fixed layer offsets of the shared
 *           main-executable data section: 0x80181BAC and 0x80181BB0 with route
 *           3, then 0x80181BBC and 0x80181BC0 with route 5. For each probe it
 *           projects the work position by adding that offset pair to the work
 *           words 0x34 and 0x38, asks the route-parameterised position routine
 *           func_80154F78 for the sample of that point with that route, and while
 *           the scratchpad gate byte 0x1F800000 reads nonzero and the sample
 *           narrowed to 16 bits is at least 0x41, it asks the screen-space
 *           routine func_80154F28 for the same point and clears the claim byte
 *           when the signed reference halfword 0x3E is below that answer. A
 *           layer whose sample reads below the bound, or whose query a zero gate
 *           byte skips, leaves the claim byte untouched, so the two probes only
 *           ever clear it. It finishes by selecting sequence 0x42 plus the route
 *           index halved through the shared selector func_8014D8D4 and then
 *           incrementing the step counter byte 0x02. It takes no arguments, has
 *           no caller inside the payload and answers no value. The 0x20-byte
 *           frame keeps $ra at 0x18, the second projected coordinate at 0x14 and
 *           the first at 0x10 across the calls, the pointer cell is loaded before
 *           the frame is created and re-read before every block and store, and
 *           the route constants 3 and 5 are materialised in the argument
 *           register per probe, the 5 of the second probe in the branch delay
 *           slots of the first probe. The halved route index shifts logically,
 *           because the byte is a zero-extended unsigned char, so route 0
 *           selects sequence 0x42. The span runs from the prologue at
 *           0x801CF37C to the return at 0x801CF544 and its 460 bytes are bounded
 *           by the declared rows 0x801CF338 and 0x801CF548.
 *
 * Shape note: one payload comparison measured this 460-byte span byte-identical
 * to the exact lifts at 0x801CE870 of the plp457 overlay, 0x801CEE4C of plp257
 * and 0x801CFB74 of plp025, and the clone dry-run against the plp457
 * representative measured the same 460 bytes. This body holds no
 * payload-relative absolute words - its data and callees all sit in the shared
 * main executable or the scratchpad - so only the relative branch words could
 * differ, and none did. This source was seeded from that representative and
 * written against the bytes of this body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepBackPlusHalf(
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
