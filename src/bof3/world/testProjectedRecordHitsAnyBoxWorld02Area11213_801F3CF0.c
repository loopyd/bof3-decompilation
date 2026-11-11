#include "bof3/bof3.h"

/* Scale byte +9 of the 0x140-byte-stride work records based at 0x80145E90: the
 * factor its two position words are multiplied by before the sum is shifted
 * back to whole screen units. Splat binds the address through this target's
 * generated undefined_syms_auto.txt.
 * @source 0x80145E99 @kind data
 */
extern u8 D_80145E99[];

/* Word +0xC of the same records: the first position word that the scale byte
 * multiplies. Addressed through a byte offset so the word keeps its own symbol
 * base and re-materialises $at per access, as the original does.
 * @source 0x80145E9C @kind data
 */
extern u8 D_80145E9C[];

/* Word +0x10 of the same records: the second position word, scaled by the same
 * byte. */
extern u8 D_80145EA0[]; /* @source 0x80145EA0 @kind data */

/* Word +0x34 of the same records: the whole-unit base the first scaled word is
 * added to before the 16-fraction shift. */
extern u8 D_80145EC4[]; /* @source 0x80145EC4 @kind data */

/* Word +0x38 of the same records: the base the second scaled word is added
 * to. */
extern u8 D_80145EC8[]; /* @source 0x80145EC8 @kind data */

/* Record count of the same table; read once into a signed local, so the
 * original's own loop test is signed. */
extern u8 D_80146254; /* @source 0x80146254 @kind data */

/* Four 4-byte box rows in this overlay's own data, one byte symbol per field:
 * the left edge, the top edge, the half width and the half height of the box a
 * projected record position is tested against. */
extern u8 D_801F492C[]; /* @source 0x801F492C @kind data */
extern u8 D_801F492D[]; /* @source 0x801F492D @kind data */
extern u8 D_801F492E[]; /* @source 0x801F492E @kind data */
extern u8 D_801F492F[]; /* @source 0x801F492F @kind data */

/* The frame slot is a reconstruction of a declaration the original translation
 * unit carried but whose emitted code never references it: this body needs no
 * storage of its own, the prologue is a bare addiu $sp,-0x8 and the epilogue a
 * bare addiu $sp,0x8, and no instruction in the 0x124 original bytes touches a
 * $sp offset. Declaring one 8-byte local restores that reservation; the name
 * and type are a reconstruction choice constrained only by the slot size. Same
 * recorded shape as the frame slots in
 * findWorkRecordIndexAtPositionWorld02Area08613_801F34F8 and
 * noopWorld03Area13913_801F3A7C. Flagged for independent review; removing the
 * declaration leaves only the frame-size residual.
 */
/* @source 0x801F3CF0
 * @behavior Overlay step body with no caller inside this payload: it scans the
 *           byte record count at 0x80146254 entries of the 0x140-byte-stride
 *           work table based at 0x80145E90 and reports whether any record's
 *           projected screen position falls inside one of the four 4-byte box
 *           rows at 0x801F492C. No live flag is consulted. For each record the
 *           +0xC and +0x10 words are multiplied by its +9 scale byte, the
 *           products are added to its +0x34 and +0x38 words, and each sum is
 *           shifted arithmetically by 16 into a whole-unit coordinate; the
 *           record is then tested against every box row in turn, whose bytes
 *           are the left edge, the top edge, the half width and the half
 *           height, and it matches when both absolute differences are below
 *           the matching half extent. Returns 1 on the first match and 0 when
 *           the count is zero or nothing matches. The row stride is a byte
 *           offset, no call is made, and the frame holds only a reconstruction
 *           slot no instruction touches.
 * @status partial
 * @match 75.34
 * @residual Every emitted instruction is the original's, in the original's
 *           order, at the original's 292 bytes; the only differing operands
 *           are register names. The original keeps the record index in $a3,
 *           the box index in $a1, the first projected coordinate in $t1 and
 *           the second in $t0; this build holds them in $t1, $a0, $t0 and $a3.
 *           Declaration order, loop form, signedness, shift form, product
 *           temporaries and one or two base temporaries were measured and none
 *           of them move that assignment, so the remainder is the compiler's
 *           local-alloc tie-break rather than a statement-shape difference.
 *           The unqualified source form of the two projections scores 52/73;
 *           holding the two base words in locals scores 55/73 and is the shape
 *           retained here.
 */
s32 testProjectedRecordHitsAnyBoxWorld02Area11213_801F3CF0(void) {
  u32 unusedFrameSlot[2];
  s32 count;
  s32 i;
  s32 limit;
  s32 offset;
  s32 scale;
  s32 sx;
  s32 sy;
  s32 j;
  s32 left;
  s32 top;
  s32 halfWidth;
  s32 dx;
  s32 dy;
  s32 baseX;
  s32 baseY;

  (void)unusedFrameSlot;
  count = D_80146254;
  i = 0;
  if (count > 0) {
    limit = count;
    offset = 0;
    do {
      scale = D_80145E99[offset];
      sx = *(s32 *)(D_80145E9C + offset) * scale;
      baseX = *(s32 *)(D_80145EC4 + offset);
      baseY = *(s32 *)(D_80145EC8 + offset);
      sy = *(s32 *)(D_80145EA0 + offset) * scale;
      sx = (baseX + sx) >> 16;
      sy = (baseY + sy) >> 16;
      j = 0;
      do {
        left = D_801F492C[j];
        top = D_801F492D[j];
        halfWidth = D_801F492E[j];
        dx = sx - left;
        if (dx < 0) {
          dx = -dx;
        }
        dy = sy - top;
        if (dy < 0) {
          dy = -dy;
        }
        if (dx < halfWidth && dy < D_801F492F[j]) {
          return 1;
        }
        j += 4;
      } while (j < 0x10);
      i++;
      offset += 0x140;
    } while (i < limit);
  }
  return 0;
}
