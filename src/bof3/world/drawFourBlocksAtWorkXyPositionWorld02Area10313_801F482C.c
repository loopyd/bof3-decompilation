#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared primitive cursor of the main executable. Splat binds the address
 * through this target's generated undefined_syms_auto.txt.
 * @source 0x8014598C @kind bss
 */
extern u8* g_PrimCursor;

/* Drawing pass of the main executable: it links the 0x18-byte packet at the
 * cursor into ordering-table entry six and advances the cursor past it. */
extern void func_8014E5A0(s32 ot_index,
                          s32 packet_size); /* @source 0x8014E5A0 */

/* @source 0x801F482C
 * @behavior Draws the four-block backdrop this overlay hands every closing
 *           drawing pass. It reads the work record published at the scratchpad
 *           pointer cell 0x1F800044 and turns the record's fixed-point position
 *           words 0x34 and 0x38 into the two split sizes the blocks are cut
 *           with: the width is the low five bits of the integer x coordinate
 *           (the upper halfword of word 0x34) flipped about 0x10 and halved,
 *           and the height is the low five bits of the integer y coordinate
 *           (the upper halfword of word 0x38). The four blocks then partition a
 *           0x10-wide by 0x20-high area at the split point (width, height):
 *           0x170/0x1A0 with size (width, height), 0x170 + width/0x1A0 with
 *           size (0x10 - width, height), 0x170/0x1A0 + height with size
 *           (width, 0x20 - height) and 0x170 + width/0x1A0 + height with size
 *           (0x10 - width, 0x20 - height). Each block is handed to the LIBGPU
 *           draw-area mover SetDrawMove with the primitive published at
 *           g_PrimCursor and the fixed draw-area origins (0x270 - width,
 *           0x120 - height), (0x260, 0x120 - height), (0x270 - width, 0x100)
 *           and (0x260, 0x100), and each 0x18-byte packet is queued through
 *           func_8014E5A0(6, 0x18). The primitive pointer is re-read for every
 *           packet, so the four call groups share no loaded pointer. Takes no
 *           arguments, returns nothing and uses a 0x38-byte frame that keeps
 *           $ra and $s0-$s6: the split sizes stay in $s0 and $s2 across the
 *           call groups and the recurring rect origin and draw-area constants
 *           0x170, 0x1A0, 0x270 and 0x120 sit in $s6, $s3, $s4 and $s1. The
 *           meaning of the draw-area origins and of the five-bit split stays
 *           described by value.
 * @status partial
 * @match 91.49
 * @residual The original recomputes the (0x270 - width) argument into $a2 for
 *           both the first and the third packet, so it keeps the 0x270 constant
 *           in $s4 across the whole body and needs no extra packet setup. This
 *           candidate CSEs the two identical arguments across the two calls:
 *           the merged value takes the freed 0x270 register and costs one extra
 *           `addu $a2,<value>,zero` in the first group and a copy instead of the
 *           recompute in the third (94 instructions against the original's 93).
 *           Measured allocations: the 0x120 - height argument, whose two sites
 *           are one call apart, is CSEd into $s1 by both bodies.
 */
void drawFourBlocksAtWorkXyPositionWorld02Area10313_801F482C(void) {
  RECT rect;
  u8* work;
  s32 width;
  s32 height;

  work = D_1F800044;
  width = ((*(s32*)(work + 0x34) >> 12) & 0x1F ^ 0x10) >> 1;
  height = (*(s32*)(work + 0x38) >> 12) & 0x1F;

  rect.x = 0x170;
  rect.y = 0x1A0;
  rect.w = width;
  rect.h = height;
  SetDrawMove((DR_MOVE*)g_PrimCursor, &rect, 0x270 - width, 0x120 - height);
  func_8014E5A0(6, 0x18);

  rect.x = 0x170 + width;
  rect.y = 0x1A0;
  rect.w = 0x10 - width;
  rect.h = height;
  SetDrawMove((DR_MOVE*)g_PrimCursor, &rect, 0x260, 0x120 - height);
  func_8014E5A0(6, 0x18);

  rect.x = 0x170;
  rect.y = 0x1A0 + height;
  rect.w = width;
  rect.h = 0x20 - height;
  SetDrawMove((DR_MOVE*)g_PrimCursor, &rect, 0x270 - width, 0x100);
  func_8014E5A0(6, 0x18);

  rect.x = 0x170 + width;
  rect.y = 0x1A0 + height;
  rect.w = 0x10 - width;
  rect.h = 0x20 - height;
  SetDrawMove((DR_MOVE*)g_PrimCursor, &rect, 0x260, 0x100);
  func_8014E5A0(6, 0x18);
}
