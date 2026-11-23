#include "bof3/bof3.h"

/* Scratchpad deck of the four 8-byte vectors the quad corners are built in and
 * handed to RotTransPers4, based at 0x1F800014.
 * @source 0x1F800014 @kind unknown
 */
extern SVECTOR D_1F800014[];

/* Shared primitive cursor of the main executable, the FT4 packet this helper
 * seeds and submits.
 * @source 0x8014598C @kind bss
 */
extern u8* g_PrimCursor;

/* Fixed-address main-executable helpers: the primitive texture-state filler and
 * the output-slot appender. */
void func_80155560(u32 command, void* primitive, s32 mode); /* @source 0x80155560 */
void func_8014E5A0(u32 ot_index, u32 primitive_size); /* @source 0x8014E5A0 */

/* @source 0x801F4160
 * @behavior Overlay quad helper. It seeds the shared primitive cursor
 *           g_PrimCursor as a textured quad with SetPolyFT4 and clears its
 *           shade texture, then fills the four scratchpad vectors at
 *           0x1F800014 with a square centred on the fixed point
 *           (-0x3500, -0x1F00, -0x80) whose half-extent is the first argument
 *           shifted left by two: the low-x corners 0/2 take
 *           -0x3500 - extent and the high-x corners 1/3 take extent - 0x3500,
 *           the low-y corners 0/1 take -0x1F00 - extent and the high-y corners
 *           2/3 take extent - 0x1F00, and every z is -0x80. RotTransPers4
 *           projects those four vectors into the primitive's four vertex
 *           pairs, func_80155560 fills the packet's texture state from
 *           0x25100100 plus the second argument's 5-bit page field shifted left
 *           by sixteen, and the 0x28-byte packet is linked into output slot 5
 *           through func_8014E5A0. Its only caller is this target's progress
 *           handler 0x801F405C, which passes the same work byte twice as the
 *           extent and the page field. Takes the extent and the page field,
 *           returns nothing.
 * @status partial
 * @match 96.20
 * @residual live gate 76/79 instructions, 316 bytes against 312 original. The
 *           scratchpad square, the primitive seeding, the RotTransPers4
 *           argument staging, the page-field command and the output-slot
 *           append match instruction for instruction; the frame is 0x40 with
 *           $ra at +0x3C, $s2 at +0x38, $s1 at +0x34 and $s0 at +0x30. First
 *           differs at +0x0070: the v[1].vx halfword store at deck +0x08 is
 *           expanded as `lui $at,0x1F80` plus `sh $a3,0x1C($at)` and scheduled
 *           after the RotTransPers4 staging, where the original keeps the deck
 *           base in $a0 and stores `sh $a3,0x8($a0)` beside its pair. The other
 *           three corner stores do use $a0, so this is the reload address-form
 *           choice for one store rather than a shape difference: eight clean-C
 *           variants (s32 and s16 edge locals, chained and separate stores in
 *           both pair orders, byte-offset FIELD_REF stores, an
 *           integer-constant scratchpad base, a second deck pointer and an
 *           inline edge expression) all measured 312->316 (+4) or worse, and
 *           the chained pairing is what keeps three of the four stores
 *           base-relative.
 */
void drawSizedTexturedQuadAtFixedCenterWithPageFieldWorld02Area10814_801F4160(s32 size, s32 page) {
  SVECTOR*  v;
  POLY_FT4* prim;
  s32       extent;
  s16       lo;
  s16       hi;
  s16       ylo;
  s16       yhi;
  long      depth;
  long      flag;

  prim = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(prim);
  SetShadeTex(prim, 0);

  extent = size << 2;
  lo = -0x3500 - extent;
  hi = extent - 0x3500;
  ylo = -0x1F00 - extent;
  yhi = extent - 0x1F00;

  v = D_1F800014;
  v[0].vx = v[2].vx = lo;
  v[1].vx = v[3].vx = hi;
  SPAD_REF(s16, 0x30u) = -0x80;
  SPAD_REF(s16, 0x28u) = -0x80;
  SPAD_REF(s16, 0x20u) = -0x80;
  SPAD_REF(s16, 0x18u) = -0x80;
  SPAD_REF(s16, 0x1eu) = ylo;
  SPAD_REF(s16, 0x16u) = ylo;
  SPAD_REF(s16, 0x2eu) = yhi;
  SPAD_REF(s16, 0x26u) = yhi;

  RotTransPers4(v, v + 1, v + 2, v + 3, (long*)&prim->x0, (long*)&prim->x1,
                (long*)&prim->x2, (long*)&prim->x3, &depth, &flag);

  func_80155560(((page & 0xF8) << 16) + 0x25100100, (void*)prim, 1);
  func_8014E5A0(5, 0x28);
}
