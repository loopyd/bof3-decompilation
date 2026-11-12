#include "bof3/ui/shop00_internal.h"

/* Engine primitive initialiser of the main executable (unlifted): it writes the
 * tag and the code byte of the packet that g_PrimCursor points at. This payload
 * only calls it, and the address lies outside every EMI payload.
 * @source 0x8017AA80 @kind function */
extern void func_8017AA80(void *primitive);

/* 0x10-byte sprite packet this emitter fills: four tag bytes, the colour bytes
 * at 0x4 through 0x6 and the code byte at 0x7 that the initialiser writes, the
 * two coordinates at 0x8 and 0xA, and the two halfwords at 0xC and 0xE that the
 * 0x10-byte size and these offsets place at the PsyQ SPRT_8 texture byte pair
 * and clut word. Both of those are written as one halfword each from a 16-bit
 * argument, so they keep their 16-bit spelling. */
typedef struct {
  u8 pad_00[4];
  u8 r0;
  u8 g0;
  u8 b0;
  u8 pad_07;
  s16 x0;
  s16 y0;
  s16 unk_0c;
  s16 unk_0e;
} ShopSpritePrim;

/* @source 0x801D9160
 * @behavior Emitter of the SHOP overlay that draws one textured
 *           semi-transparent sprite through two packets at g_PrimCursor. It
 *           first asks GetTPage for the 4-bit page at vram column 0x3C0 with
 *           the semi-transparency rate taken from its own last byte argument,
 *           stores that page word into a draw-mode packet through SetDrawMode
 *           with no dithering, no texture disable and no texture window, and
 *           appends that 0xC-byte packet at ordering-table slot 1. A second
 *           read of g_PrimCursor initialises a 0x10-byte sprite packet with
 *           func_8017AA80, marks it semi-transparent, fills its colour bytes
 *           0x4 through 0x6 from the byte arguments 4, 5 and 6, its
 *           coordinates 0x8 and 0xA from arguments 0 and 1, the halfword at
 *           0xC from argument 2 and the halfword at 0xE from argument 3, and
 *           appends it at slot 1 as a 0x10-byte primitive. Both GetTPage and
 *           SetDrawMode are PsyQ calls of the main executable; the two appends
 *           are appendRenderPrim at 0x8014E5A0. No jal and no word of this
 *           payload names this span, so it is an overlay entry called from
 *           outside the payload, and its 0x40-byte frame keeps $s0 through $s7
 *           and $ra only across those four calls. One payload cmp proved its
 *           0xF4 bytes equal drawSpritePrimTpageFromAbrBate00_801D36B8 at
 *           0x801D36B8 in emi/etc/bate/00 and
 *           drawSpritePrimTpageFromAbrShisu00_801D4660 at 0x801D4660 in
 *           emi/etc/shisu/00, both of which had measured exact, so it carries
 *           no target-specific immediate and it keeps their statement order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpritePrimTpageFromAbrShop00_801D9160(s16 arg0, s16 arg1, s16 arg2, s16 arg3,
                                               u8 arg4, u8 arg5, u8 arg6, u8 arg7) {
  ShopSpritePrim* primitive;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, arg7, 0x3C0, 0), 0);
  appendRenderPrim(1, 0xC);
  primitive = (ShopSpritePrim*)g_PrimCursor;
  func_8017AA80(primitive);
  SetSemiTrans(primitive, 1);
  primitive->r0 = arg4;
  primitive->g0 = arg5;
  primitive->b0 = arg6;
  primitive->x0 = arg0;
  primitive->y0 = arg1;
  primitive->unk_0c = arg2;
  primitive->unk_0e = arg3;
  appendRenderPrim(1, 0x10);
}
