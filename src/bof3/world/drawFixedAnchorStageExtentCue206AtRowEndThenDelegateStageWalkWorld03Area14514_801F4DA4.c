#include "bof3/bof3.h"

/* Columns of the shared 0x98-byte work records of the main executable: the
 * record position words 0x34 and 0x38 at D_80147A8C and D_80147A90 and the
 * packed status word at D_80147AC4, 0x38 bytes further on. This body reads the
 * first record of every column without a stage-code scale, so each access keeps
 * its own address-derived symbol. */
extern s32 D_80147A8C; /* @source 0x80147A8C @kind data */
extern s32 D_80147A90; /* @source 0x80147A90 @kind data */
extern s32 D_80147AC4; /* @source 0x80147AC4 @kind data */

/* Shared route-step table of the main executable: entry i holds the
 * first-coordinate step at D_80181B00 and the second-coordinate step at
 * D_80181B01, two bytes per entry, so the original indexes it with the route
 * byte times two. Both loads are signed here, which is the original's lb. */
extern s8 D_80181B00[]; /* @source 0x80181B00 @kind table */
extern s8 D_80181B01[]; /* @source 0x80181B01 @kind table */

/* Stage-entry table of this overlay's trailing data blob, sixteen bytes at
 * 0x801F5CDC: four four-byte entries holding the route step index at byte 0,
 * the stage length at byte 1 and the next stage code at byte 2. The body
 * reaches the three columns through one four-byte offset, and the original
 * re-materialises each column base from its own symbol, so no record view is
 * used here. The three rows of the target map bind the addresses. */
extern u8 D_801F5CDC[]; /* @source 0x801F5CDC @kind table */
extern u8 D_801F5CDD[]; /* @source 0x801F5CDD @kind table */
extern u8 D_801F5CDE[]; /* @source 0x801F5CDE @kind table */

/* Draw request of this payload at 0x801F5190, lifted in
 * claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190.c:
 * it takes two 16.16 points and queues the record pair that spans them. */
s32 claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190(
    s32 x0, s32 y0, s32 x1, s32 y1);

/* Stage walk of this payload at 0x801F4EDC, lifted in
 * drawRecordAnchorStageExtentCue206AtRowEndThenDelegateStageWalkWorld03Area14514_801F4EDC.c:
 * it takes the elapsed step value and the stage code of the stage-entry table and
 * walks that stage until it expires. */
s32 drawRecordAnchorStageExtentCue206AtRowEndThenDelegateStageWalkWorld03Area14514_801F4EDC(
    s32 arg0, s32 arg1);

/* Shared cue request of the main executable: it takes the cue id in the low
 * half. */
void func_8015DF18(u16 arg0);

/* @source 0x801F4DA4
 * @behavior Second stage of the chained front-end effect, reached from the
 *           front-end step at 0x801F4CF0 for step values from five on. It takes
 *           the step value, subtracts five, and uses bits 9 and 10 of the first
 *           shared work record status word as a four-byte offset into the
 *           payload stage-entry table at 0x801F5CDC. Entry byte 0 indexes the
 *           shared route-step pair D_80181B00 and D_80181B01, whose signed
 *           steps the original shifts left by sixteen to 16.16; entry byte 1 is
 *           the stage length and entry byte 2 the next stage code. The body
 *           clamps the elapsed value to that length and queues the draw request
 *           0x801F5190 for the pair from the fixed 16.16 point 0x3D8000/
 *           0x638000 to the shared record position advanced by each step times
 *           the clamped value. It answers 1 while the elapsed value stays under
 *           the length and 1 again when the entry carries no next stage code,
 *           queues cue 0x206 at the step that reaches the length, and otherwise
 *           hands the elapsed value and the next stage code to the stage walk at
 *           0x801F4EDC and answers its narrowed result. The 0x20-byte frame
 *           keeps $ra, $s0 and $s1.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Name evidence: the caller at 0x801F4CF0 already documents this address as the
 * payload stage drawer it delegates to from step five on, and the draw request
 * it reaches is the same 0x801F5190 pair claim the exact lift
 * claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190
 * performs; 0x3D8000 and 0x638000 are two of the fixed 16.16 coordinates of the
 * window that lift and the caller record. The four-byte rows this body selects
 * hold 0, 5, 1 / 2, 7, 0 / 4, 7, 0 / 6, 5, 2, so byte 0 is an even route index
 * into the shared route-step pair, byte 1 a length and byte 2 a next-stage code;
 * the review-pending lift of 0x801F504C records the same three columns for the
 * neighbouring table of the same blob, and the walker at 0x801F4EDC reaches that
 * body, which is why the tail hands it the next code. Cue 0x206 is the cue the
 * caller queues at its own row end. The name therefore states what the body
 * does: it draws this stage extent from the fixed anchor, cues at the row end
 * and then delegates the walk.
 *
 * Shape notes from the original bytes: the status word is read as a signed
 * word and the four-byte offset is its seventh-bit shift masked with 0xC, so the
 * offset is spelled as one masked shift rather than a doubled index. The step
 * value narrows once into a halfword local that stays live across the call, and
 * the stage length is a word local loaded once and kept live from that load to
 * the second product, which is what makes the two selections `move` of the
 * loaded length rather than a byte extension. Each product needs its own
 * selection local of its own start value, so the first is a copy of the elapsed
 * value while the second reuses it, and both mask the selected value to a
 * halfword at the multiply. The comparisons against the halfword step stay
 * unsigned through the word length local, so no cast is written. Product one is
 * summed with D_80147A8C into a local and copied to the third argument at the
 * call, product two fills the call delay slot with its own sum. The tail
 * re-reads the length and next-code columns after the call, once per use, so
 * those cells stay array reads.
 */
u8 drawFixedAnchorStageExtentCue206AtRowEndThenDelegateStageWalkWorld03Area14514_801F4DA4(
    u32 arg0) {
  s32 elapsed;
  u16 t;
  u32 off;
  u32 limit;
  s32 mx;
  s32 my;
  s32 dx;
  s32 dy;
  s32 x;
  s32 y;

  elapsed = (s32)arg0 - 5;
  t = (u16)elapsed;
  off = ((s32)D_80147AC4 >> 7) & 0xC;
  limit = D_801F5CDD[off];
  dx = (s8)D_80181B00[D_801F5CDC[off] * 2] << 16;
  mx = elapsed;
  if (limit < t) {
    mx = limit;
  }
  x = D_80147A8C + dx * (u16)mx;
  dy = (s8)D_80181B01[D_801F5CDC[off] * 2] << 16;
  my = elapsed;
  if (limit < t) {
    my = limit;
  }
  y = D_80147A90 + dy * (u16)my;
  claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190(
      0x3D8000, 0x638000, x, y);
  if (t < D_801F5CDD[off]) {
    return 1;
  }
  if (D_801F5CDE[off] == 0) {
    return 1;
  }
  if (t == D_801F5CDD[off]) {
    func_8015DF18(0x206);
  }
  return drawRecordAnchorStageExtentCue206AtRowEndThenDelegateStageWalkWorld03Area14514_801F4EDC(
      t, D_801F5CDE[off]);
}
