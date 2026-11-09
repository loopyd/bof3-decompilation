#include "bof3/bof3.h"

/* Stage-entry table pair of this overlay's trailing data blob at 0x801F5D0C
 * and 0x801F5D1C: sixteen bytes each, four four-byte entries holding the route
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

extern StageEntry D_801F5D0C[]; /* @source 0x801F5D0C @kind table */
extern StageEntry D_801F5D1C[]; /* @source 0x801F5D1C @kind table */

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
 * it takes two 16.16 points and queues the record pair that spans them. */
s32 claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190(
    s32 x0, s32 y0, s32 x1, s32 y1);

/* @source 0x801F504C
 * @behavior One stage of a chained overlay effect, reached from the stage walk
 *           at 0x801F4EDC. It takes the stage code in its second argument,
 *           indexes the shared 0x98-byte work records with it, and reads the
 *           two-bit selector in bits 9 and 10 of that record's packed status
 *           word. The selector picks one of the four four-byte entries of the
 *           payload stage table, which the code selects as D_801F5D0C when it
 *           reads exactly 1 and D_801F5D1C otherwise. Entry byte 0 indexes the
 *           shared route-step pair D_80181B00 and D_80181B01, whose signed
 *           steps the original shifts left by sixteen to 16.16; entry byte 1 is
 *           the stage length and entry byte 2 the next stage code. From the
 *           elapsed value of the first argument less 10 the body clamps to that
 *           length, loads the record's two 16.16 position words, and queues the
 *           draw request 0x801F5190 with the record position and that position
 *           advanced by each step times the clamped elapsed value. It answers 1
 *           while the elapsed value stays under the stage length and the next
 *           stage code once it does not, so the walker keeps this stage until it
 *           expires. The 0x20-byte frame keeps $ra, $s0 and $s1.
 * @status partial
 * @match 65.43
 * @residual 53 of 81 instructions match; the build is 316 of 324 bytes.
 *           The first difference at offset 0x34 is a nop instead of the
 *           original first-argument copy to $a2. Selector allocation, table-arm
 *           delay slots, clamp conversions and product/load scheduling differ.
 *           Both coordinate products retain separate full-width clamp joins
 *           followed by conversion to u16. The candidate is review-pending;
 *           compiler-pass diagnosis and authorized permutation remain open.
 */
u8 drawStageExtentAndReturnNextStageCodeOnExpiryWorld03Area14514_801F504C(s32 arg0,
                                                                          s32 arg1) {
  StageEntry* stage;
  s32 sel;
  u16 t;
  s32 m;
  s32 off;
  s32 dx;
  s32 dy;
  s32 elapsed;
  s32 x;
  s32 y;

  sel = (*(s32*)((u8*)&D_80147AC4 + (arg1 & 0xFF) * 0x98) >> 9) & 3;
  if ((arg1 & 0xFF) != 1) {
    stage = D_801F5D1C + sel;
  } else {
    stage = D_801F5D0C + sel;
  }
  elapsed = arg0 - 0xA;
  t = (u16)elapsed;
  off = (arg1 & 0xFF) * 0x98;
  dx = D_80181B00[stage->route * 2] << 16;
  m = elapsed;
  if (stage->length < t) {
    m = stage->length;
  }
  x = dx * (u16)m;
  dy = D_80181B01[stage->route * 2] << 16;
  m = elapsed;
  if (stage->length < t) {
    m = stage->length;
  }
  y = dy * (u16)m;
  claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190(
      *(s32*)((u8*)&D_80147A8C + off), *(s32*)((u8*)&D_80147A90 + off),
      *(s32*)((u8*)&D_80147A8C + off) + x,
      *(s32*)((u8*)&D_80147A90 + off) + y);
  return t < stage->length ? 1 : stage->next;
}
