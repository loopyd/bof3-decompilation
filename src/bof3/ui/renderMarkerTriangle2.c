#include "bof3/bof3.h"

/* Engine helpers this overlay emitter calls: 0x8017A954 is the POLY_F3
 * initialiser that stamps the packet the shared cursor points at, and
 * 0x8014E5A0 appends a packet of the given byte size to an ordering-table
 * slot. Both live in the main executable, outside every EMI payload, and no
 * word of this payload names them. */
extern void func_8017A954(void *prim); /* @source 0x8017A954 */
extern void func_8014E5A0(u32 ot_index, u32 primitive_size); /* @source 0x8014E5A0 */

/* The shared primitive cursor the engine publishes for the overlay emitters. */
extern u8 *g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Shared frame counter owned by the main exe; its low bits select the marker
 * tint. */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* @source 0x801D88A4
 * @behavior Emits one flat-shaded marker triangle at the shared primitive
 *           cursor. It stamps that cursor through the POLY_F3 initialiser at
 *           0x8017A954, writes the apex (x, y) and the two base corners
 *           (x - 4, y - 4) and (x + 4, y - 4) as the six halfwords at offsets
 *           0x08 to 0x12, writes 0 to the tint bytes at offsets 0x04 and 0x06
 *           with the green byte at 0x05, then appends the 0x14-byte packet
 *           through func_8014E5A0 with size class 1. A nonzero third byte
 *           forces the green byte to the full 0xFF; a zero third byte takes
 *           bits 1 and 2 of the shared frame counter 0x80143E6C, scaled by 32
 *           and offset by 0x3F, inverted while bit 3 of that counter is clear,
 *           so an unselected marker blinks. Neither helper address lies inside
 *           this payload and no word of the payload names this span, so its
 *           three argument roles stay structural. No global state is written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderMarkerTriangle2(s32 x, s16 y, u8 selected) {
  u32 prim;
  u32 flags;
  u32 color;

  if (selected == 0) {
    flags = D_80143E6C;
    if (flags & 8) {
      color = ((flags & 6) << 5) | 0x3F;
    } else {
      color = ((~flags & 6) << 5) | 0x3F;
    }
  } else {
    color = 0xFF;
  }
  prim = (u32)g_PrimCursor;
  func_8017A954((void *)prim);
  *(s16 *)(prim + 12) = x - 4;
  *(s16 *)(prim + 8) = x;
  *(s16 *)(prim + 10) = y;
  *(s16 *)(prim + 14) = y - 4;
  *(s16 *)(prim + 16) = x + 4;
  *(s16 *)(prim + 18) = y - 4;
  *(u8 *)(prim + 4) = 0;
  *(u8 *)(prim + 5) = color;
  *(u8 *)(prim + 6) = 0;
  func_8014E5A0(1, 0x14);
}
