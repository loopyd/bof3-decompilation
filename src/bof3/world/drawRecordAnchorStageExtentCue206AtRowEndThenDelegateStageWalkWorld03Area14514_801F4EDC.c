#include "bof3/bof3.h"

/* Stage-entry table pair of this overlay's trailing data blob at 0x801F5CEC
 * and 0x801F5CFC: sixteen bytes each, four four-byte entries holding the route
 * step index at byte 0, the stage length at byte 1 and the next stage code at
 * byte 2. The body picks the pair with the stage code and indexes the selected
 * one with a two-bit field of the work record. The target generated
 * undefined_syms_auto.txt binds both addresses. */
typedef struct StageEntry {
  u8 route;
  u8 length;
  u8 next;
  u8 pad;
} StageEntry;

extern StageEntry D_801F5CEC[]; /* @source 0x801F5CEC @kind table */
extern StageEntry D_801F5CFC[]; /* @source 0x801F5CFC @kind table */

/* Shared route-step table of the main executable: entry i holds the
 * first-coordinate step at D_80181B00 and the second-coordinate step at
 * D_80181B01, two bytes per entry, so the original indexes it with the route
 * byte times two. Both loads are signed here, which is the original's lb. */
extern s8 D_80181B00[]; /* @source 0x80181B00 @kind table */
extern s8 D_80181B01[]; /* @source 0x80181B01 @kind table */

/* Columns of the shared 0x98-byte work records of the main executable: the
 * record position words 0x34 and 0x38 at D_80147A8C and D_80147A90 and the
 * packed status word at D_80147AC4, 0x38 bytes further on. Every access scales
 * the stage code by 0x98 against one of these columns, so each keeps its own
 * address-derived symbol. */
extern s32 D_80147A8C; /* @source 0x80147A8C @kind data */
extern s32 D_80147A90; /* @source 0x80147A90 @kind data */
extern s32 D_80147AC4; /* @source 0x80147AC4 @kind data */

/* Draw request of this payload at 0x801F5190, lifted in
 * claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190.c:
 * it takes two 16.16 points and queues the record pair that spans them. It is
 * declared with a word result here because the original narrows the answer of
 * the stage it delegates to, not this one. */
s32 claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190(
    s32 x0, s32 y0, s32 x1, s32 y1);

/* Stage of this payload at 0x801F504C, lifted in
 * drawStageExtentAndReturnNextStageCodeOnExpiryWorld03Area14514_801F504C.c: it
 * takes the elapsed step value and its stage code, draws that stage from the
 * advanced work record position and answers 1 until the stage expires. The
 * original narrows that answer with an andi, so it is declared with a word
 * result here. */
s32 drawStageExtentAndReturnNextStageCodeOnExpiryWorld03Area14514_801F504C(
    s32 arg0, s32 arg1);

/* Shared cue request of the main executable: it takes the cue id in the low
 * half. */
void func_8015DF18(u16 arg0);

/* @source 0x801F4EDC
 * @behavior Third stage of the chained front-end effect, reached from the
 *           fixed-anchor stage drawer at 0x801F4DA4 with the stage code that
 *           drawer passed it. It takes the elapsed step value and the stage code,
 *           subtracts five from the elapsed value, and indexes the shared
 *           0x98-byte work records with the code to read that record packed
 *           status word. Bits 9 and 10 of the word pick one of the four
 *           four-byte entries of the payload stage-entry table, which the code
 *           selects as D_801F5CEC when it reads exactly one and D_801F5CFC
 *           otherwise. Entry byte 0 indexes the shared route-step pair
 *           D_80181B00 and D_80181B01, whose signed steps the original shifts
 *           left by sixteen to 16.16; entry byte 1 is the stage length and entry
 *           byte 2 the next stage code. The body clamps the elapsed value to that
 *           length and queues the draw request 0x801F5190 for the pair from the
 *           record position to that position advanced by each step times the
 *           clamped value. It answers 1 while the elapsed value stays under the
 *           length and 1 again when the entry carries no next stage code, queues
 *           cue 0x206 at the step that reaches the length, and otherwise hands
 *           the elapsed value and the next stage code to the stage at 0x801F504C
 *           and answers its narrowed result. The 0x20-byte frame keeps $ra, $s0
 *           and $s1.
 * @status partial
 * @match 69.57
 * @residual 64 of 92 instructions match at the original 368 bytes. The first
 *           difference at offset 0x34 is the missing entry copy of the first
 *           argument: the original keeps the elapsed value in $a2 and lets the
 *           two-bit selector take $a0, while this candidate lets the selector
 *           take $a2 and keeps the elapsed value in $a0. The table-arm delay
 *           slots and pointer-add join differ too, as do the record-offset copy
 *           and coordinate register allocation. The draw request, tail tests
 *           and epilogue agree. The candidate is review-pending.
 *
 * Name evidence: the caller at 0x801F4DA4 already documents 0x801F4EDC as the
 * stage walk it delegates to at its row end, and the body differs from that
 * exact caller in one respect: it takes the stage code as well, anchors the
 * drawn extent at that code's shared work record position instead of the fixed
 * 16.16 point 0x3D8000/0x638000, and delegates in turn to the stage at
 * 0x801F504C. Table D_801F5CEC holds 07 0b 00 / 01 02 00 / 03 0d 00 / 05 0a 02
 * and D_801F5CFC holds 01 0a 01 / 03 0d 00 / 05 02 00 / 07 0b 00, so byte 0 is an
 * even route index into the shared route-step pair, byte 1 a length and byte 2 a
 * next stage code, the same three columns the neighbouring tables at 0x801F5CDC
 * and 0x801F5D0C/0x801F5D1C carry. Cue 0x206 is the cue the caller and the
 * neighbour at 0x801F504C queue at their row end.
 *
 * Shape notes from the original bytes: the stage code narrows once into each
 * scale, so the 0x98 stride is spelled from the masked code at every use rather
 * than through one record view. The selector is the ninth-bit shift of the
 * status word masked to two bits and then scaled by the four-byte entry, so it
 * stays an integer index into the selected table, and the original computes it
 * once before the code test and scales it inside each arm. The elapsed value
 * narrows once into a halfword local that keeps every comparison against the
 * stage length unsigned, and the length loads once into a word local so the
 * clamp assigns it without a byte extension. Each of the two products re-runs
 * the clamp with its own local initialised from the elapsed value, and the
 * position word that seeds the pair loads once while the other fills the call
 * delay slot. The tail re-reads the length and next-code columns after the call,
 * once per use.
 */
u8 drawRecordAnchorStageExtentCue206AtRowEndThenDelegateStageWalkWorld03Area14514_801F4EDC(
    s32 arg0, s32 arg1) {
  StageEntry* stage;
  s32 sel;
  s32 off;
  s32 elapsed;
  u16 t;
  u32 limit;
  s32 mx;
  s32 my;
  s32 dx;
  s32 dy;
  s32 x;
  s32 y;

  sel = (*(s32*)((u8*)&D_80147AC4 + (arg1 & 0xFF) * 0x98) >> 9) & 3;
  if ((arg1 & 0xFF) != 1) {
    stage = D_801F5CFC + sel;
  } else {
    stage = D_801F5CEC + sel;
  }
  elapsed = arg0 - 5;
  t = (u16)elapsed;
  off = (arg1 & 0xFF) * 0x98;
  limit = stage->length;
  dx = D_80181B00[stage->route * 2] << 16;
  mx = elapsed;
  if (limit < t) {
    mx = limit;
  }
  x = *(s32*)((u8*)&D_80147A8C + off) + dx * (u16)mx;
  dy = D_80181B01[stage->route * 2] << 16;
  my = elapsed;
  if (limit < t) {
    my = limit;
  }
  y = *(s32*)((u8*)&D_80147A90 + off) + dy * (u16)my;
  claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190(
      *(s32*)((u8*)&D_80147A8C + off), *(s32*)((u8*)&D_80147A90 + off), x, y);
  if (t < stage->length) {
    return 1;
  }
  if (stage->next == 0) {
    return 1;
  }
  if (t == stage->length) {
    func_8015DF18(0x206);
  }
  return drawStageExtentAndReturnNextStageCodeOnExpiryWorld03Area14514_801F504C(
      t, stage->next);
}
