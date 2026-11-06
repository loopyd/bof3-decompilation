#include "bof3/bof3.h"

/* Byte +0 of every 0x140-byte-stride local work record based at 0x80145E90:
 * nonzero marks a live record. Splat binds the address through this target's
 * generated undefined_syms_auto.txt.
 * @source 0x80145E90 @kind data
 */
extern u8 D_80145E90[];

/* Byte +9 of the same records: the scale its two position words are multiplied
 * by before they are compared.
 * @source 0x80145E99 @kind data
 */
extern u8 D_80145E99[];

/* Word +0xC of the same records: the first position word. Addressed through a
 * byte offset so each word keeps its own symbol base and re-materialises $at
 * per access, as the original does.
 * @source 0x80145E9C @kind data
 */
extern u8 D_80145E9C[];

/* Word +0x10 of the same records: the second position word, scaled by the same
 * byte. */
extern u8 D_80145EA0[]; /* @source 0x80145EA0 @kind data */

/* Word +0x34 of the same records: the base the scaled first position word is
 * added to. */
extern u8 D_80145EC4[]; /* @source 0x80145EC4 @kind data */

/* Word +0x38 of the same records: the base the scaled second position word is
 * added to. */
extern u8 D_80145EC8[]; /* @source 0x80145EC8 @kind data */

/* Byte +0x70 of the same records: the box radius in half world units, so it is
 * compared as (radius + 2) << 15.
 * @source 0x80145F00 @kind data
 */
extern u8 D_80145F00[];

/* Record count of the same table; the shared map binds the address. */
extern u8 D_80146254;

/* The frame slot is a reconstruction of a declaration the original translation
 * unit carried but whose emitted code never references it: this body needs no
 * storage of its own, the prologue is a bare addiu $sp,-0x8 and the epilogue a
 * bare addiu $sp,0x8, and no instruction in the 0x10C original bytes touches a
 * $sp offset. Declaring one 8-byte local restores that reservation; the name
 * and type are a reconstruction choice constrained only by the slot size. Same
 * recorded shape as the frame slots in
 * noopWorld03Area13913_801F3A7C and func_801F7670. Flagged for independent
 * review; removing the declaration leaves only the frame-size residual.
 */
/* @source 0x801F34F8
 * @behavior Scans the byte record count at 0x80146254 entries of the
 *           0x140-byte-stride local work table based at 0x80145E90 and returns
 *           the index of the first live record whose scaled position lies
 *           within its own radius of the point the two arguments describe. A
 *           record is live when its leading byte is nonzero. For a live record
 *           its +0xC and +0x10 words are multiplied by its +9 scale byte, the
 *           products are added to its +0x34 and +0x38 words, and each sum is
 *           compared with the matching argument shifted to 16.16; the record
 *           matches when both absolute differences are below (radius byte
 *           +0x70 + 2) << 15. The index is returned as a signed byte, and -1
 *           when the count is zero or no record matches. The arguments are read
 *           as narrow values, no call is made, and the frame holds only a
 *           reconstruction slot no instruction touches.
 * @status partial
 * @match 59.70
 * @residual The frame and byte count agree, but the entry register assignment
 *           first differs at instruction 5 (0x801F350C): the original clears
 *           the index in $t0, while this build uses $t2. The fixed-point first
 *           argument remains in $a0 instead of $t2, and the loop bound is in
 *           $t3 instead of $t1. Both products now precede the first compare,
 *           but record loads, magnitude tests and multiply-result reads still
 *           schedule differently.
 */
s8 findWorkRecordIndexAtPositionWorld02Area08613_801F34F8(s16 x, s16 y) {
  u32 unusedFrameSlot[2];
  s32 i;
  s32 offset;
  s32 count;
  s32 limit;
  s32 fx;
  s32 fy;
  s32 scale;
  s32 radius;
  s32 sx;
  s32 oy;
  s32 ox;
  s32 d;

  (void)unusedFrameSlot;
  count = D_80146254;
  i = 0;
  if (count > 0) {
    fx = x << 16;
    fy = y << 16;
    limit = count;
    offset = 0;
    do {
      if (D_80145E90[offset] != 0) {
        scale = D_80145E99[offset];
        sx = *(s32 *)(D_80145E9C + offset) * scale;
        /* Both products consume the same scale byte. Its value is dead after
         * the second multiply, so keep that product in the same local. */
        scale = *(s32 *)(D_80145EA0 + offset) * scale;
        oy = *(s32 *)(D_80145EC8 + offset);
        ox = *(s32 *)(D_80145EC4 + offset);
        radius = (D_80145F00[offset] + 2) << 15;
        d = ox + sx - fx;
        if (d < 0) {
          d = -d;
        }
        if (d < radius) {
          d = oy + scale - fy;
          if (d < 0) {
            d = -d;
          }
          if (d < radius) {
            return i;
          }
        }
      }
      i++;
      offset += 0x140;
    } while (i < limit);
  }
  return -1;
}
