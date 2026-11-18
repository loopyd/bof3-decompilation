#include "bof3/bof3.h"

/* Flat-line packet initialiser of the engine: it writes the tag word and the
 * code byte of the packet the shared cursor points at. Its address lies outside
 * every EMI payload, and no word of this payload names it. */
extern void func_8017AA80(void *primitive); /* @source 0x8017AA80 */

/* Primitive append helper of the engine: appends the packet the shared cursor
 * holds to the given ordering-table slot with the given byte size. */
extern void func_8014E5A0(s32 ot_index,
                          s32 packet_size); /* @source 0x8014E5A0 */

/* The shared primitive cursor the engine publishes for the overlay emitters. */
extern u8 *g_PrimCursor; /* @source 0x8014598C @kind bss */

/* @source 0x801D2E60
 * @behavior Draws one mid-grey horizontal line primitive through the shared
 *           primitive cursor. It seeds the 0x10-byte flat line packet at
 *           g_PrimCursor through func_8017AA80, writes 0x80 into the three
 *           colour bytes at 0x04 to 0x06, places the first endpoint at the two
 *           structural arguments (x, then y) and the second endpoint eight
 *           pixels further along x at the same y, and appends the packet as a
 *           0x10-byte primitive at ordering-table slot 1 through
 *           func_8014E5A0. The packet is left fully opaque: nothing here marks
 *           it semi-transparent. Neither helper address lies inside this EMI
 *           payload and no word of the payload names this span, so its two
 *           argument roles stay structural. Its 0x20-byte frame keeps $s0 to
 *           $s2 and $ra across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawGreyHorizontalLinePrim(s16 x, s16 y) {
  LINE_F2 *primitive;

  primitive = (LINE_F2 *)g_PrimCursor;
  func_8017AA80(primitive);
  primitive->x0 = x;
  primitive->x1 = x + 8;
  primitive->y0 = y;
  primitive->y1 = y;
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  func_8014E5A0(1, 0x10);
}
