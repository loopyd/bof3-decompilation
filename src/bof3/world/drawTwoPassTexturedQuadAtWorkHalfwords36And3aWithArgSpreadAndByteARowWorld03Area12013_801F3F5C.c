#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell this overlay publishes and every record
 * handler of the target reads. The original materialises the cell with `lui`
 * plus an offset load for every access, so the body keeps one pointer local per
 * use.
 * @source 0x1F800044 @kind bss
 */
extern u8* D_1F800044;

/* The four scratchpad vectors at 0x1F800014 that RotTransPers4 projects into
 * the quad's four corner pairs.
 * @source 0x1F800014 @kind data
 */
extern SVECTOR D_1F800014[];

/* Shared primitive cursor of the main executable, which the draw-mode packet
 * and the textured quad are both built at.
 * @source 0x8014598C @kind bss
 */
extern u8* g_PrimCursor;

/* Shared graphic-type query used for the draw-mode texture page. */
/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* The draw-mode, textured-quad and corner-projection services are declared by
 * the PsyQ headers this target already includes; only their roles are noted
 * here. SetDrawMode stamps the 0x0C-byte packet at the cursor with both on-screen
 * flags clear and the given texture page, SetPolyFT4 tags a textured quad and
 * SetShadeTex selects flat shading for it, and RotTransPers4 maps the four
 * scratchpad vectors into the four screen pairs while homing the depth and flag
 * words. */

/* Fixed-point vector-length helper of the main executable: it takes the packed
 * corner pair and returns the corner's distance as a signed halfword, which is
 * what the body truncates before halving it into the two leading depths. */
/* @source 0x8015477C */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* Texture-state filler of the main executable: it writes the packed texture
 * page, texture window and colour lookup word into the packet at the cursor
 * and starts its texture-state run. */
/* @source 0x80155560 */
extern void func_80155560(u32 arg0, void* arg1, s32 arg2);

/* Primitive linker: it resolves the record position pair 0x34/0x38 through the
 * shared cell-size pair, links the packet at the cursor with the given kind and
 * advances the cursor by the given size.
 * @source 0x80155A08
 */
extern void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F3F5C
 * @behavior Two-pass textured-quad drawer of the world03/area120 byte-2
 *           handlers 0x801F3B28, 0x801F3B84 and 0x801F3DF0, which run it once
 *           per frame beside their own state step.
 *           It first advances work record byte 0x0A, read through the
 *           scratchpad pointer cell 0x1F800044, by the third argument and
 *           wraps that byte back below 0x0C with one subtraction, then halves
 *           the wrapped byte and keeps that row pair on its own frame: the
 *           first row is the halved byte and the second is that halved byte
 *           plus two, wrapped back below six with one subtraction of four. The
 *           two rows are then biased by the first argument to form the two
 *           per-pass texture-state words, the caller's value sitting in their
 *           high halfword and 0xBF009111 in the low one.
 *           Each of the two passes emits one 0x0C-byte DR_MODE and one
 *           0x28-byte POLY_FT4 on the shared primitive cursor, both linked
 *           through func_80155A08 with kind 1; the draw mode takes texture page
 *           0x225 for graph types 1 and 2 and 0x95 otherwise, with both flags
 *           clear and no texture window.
 *           The quad's x column comes from the signed work halfword 0x36 and
 *           its y row from the work halfword 0x3A, each folded as
 *           ((halfword - 0x80) << 7) + 0x10. The first pass spreads the four
 *           corners over y by plus or minus the second argument and pulls the
 *           two leading corners down by the fourth argument, leaving the x
 *           column flat; the second pass spreads the x column by plus or minus
 *           the second argument and leaves the y row flat, pulling the two
 *           leading corners down by the fourth argument instead. The two
 *           leading depths are -func_8015477C over the record position pair
 *           halved and pulled up by twice the second argument plus 0xC0, the
 *           two trailing ones the same distance pulled up by 0xC0 alone.
 *           RotTransPers4 projects the four vectors into the packet's corner
 *           pairs and func_80155560 fills the packet's texture state from the
 *           pass's own biased row. Takes four arguments and returns nothing;
 *           its 0x90-byte frame keeps $ra at 0x8C, $fp at 0x88 and $s0 through
 *           $s7 below them, and the record cell is reloaded for each use.
 * @status partial
 * @match 84.78
 * @residual 195/229 original instructions match at 916->920 bytes. The entry block
 *           matches. The first mismatch is the wrap join at +0x64: the
 *           non-wrapping arm stores the byte directly in its jump delay slot
 *           instead of moving the value to the shared store. Further differences
 *           include base-register allocation and scheduling before the draw
 *           loop, the second graph-type probe's folded constant arm, deck-row
 *           branch topology and temporary allocation around the depth calls.
 */
void drawTwoPassTexturedQuadAtWorkHalfwords36And3aWithArgSpreadAndByteARowWorld03Area12013_801F3F5C(
    u32 arg0, s32 arg1, u32 arg2, s32 arg3) {
  u8* work;
  u8* entryWork;
  u8* phaseWork;
  SVECTOR* v;
  s16* vy2;
  s16* vy3;
  POLY_FT4* prim;
  u32 tex[2];
  u32 phase;
  s32 tpage;
  s32 i;
  s32 x;
  s32 y;
  s32 yOuter;
  s32 yOuterDeep;
  s32 yInner;
  s32 yInnerDeep;
  s32 z;
  long depth;
  long flag;

  entryWork = D_1F800044;
  entryWork[0x0A] += arg2;
  work = D_1F800044;
  phase = work[0x0A];
  if (phase < 0xC) {
    work[0x0A] = phase;
  } else {
    work[0x0A] = phase - 0xC;
  }
  phaseWork = D_1F800044;
  phase = phaseWork[0x0A] >> 1;
  tex[0] = phase;
  phase = phase + 2 < 6 ? phase + 2 : phase - 4;
  tex[1] = phase;
  for (i = 0; i < 2; i++) {
    tex[i] += (arg0 << 16) | 0xBF009111;
  }
  v = D_1F800014;
  vy2 = (s16*)v + 9;
  vy3 = (s16*)v + 0xD;
  work = D_1F800044;
  y = (((s16)FIELD_REF(u16, work, 0x3A) - 0x80) << 7) + 0x10;
  yOuter = y + arg1;
  yOuterDeep = yOuter - arg3;
  yInner = y - arg1;
  x = (((s16)FIELD_REF(u16, work, 0x36) - 0x80) << 7) + 0x10;
  yInnerDeep = yInner - arg3;
  for (i = 0; i < 2; i++) {
    if (GetGraphType() == 1) {
      tpage = 0x225;
    } else if (GetGraphType() == 2) {
      tpage = 0x225;
    } else {
      tpage = 0x95;
    }
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
    func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                  FIELD_REF(s32, D_1F800044, 0x38), 1, 0xC);
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);
    v[0].vx = v[2].vx = (u16)(x + i * arg1);
    v[1].vx = v[3].vx = (u16)(x - i * arg1);
    if (i == 0) {
      v[0].vy = yOuterDeep;
    } else {
      v[0].vy = y - arg3;
    }
    if (i == 0) {
      vy2[0] = yOuter;
    } else {
      vy2[0] = y;
    }
    if (i == 0) {
      vy2[-4] = yInnerDeep;
    } else {
      vy2[-4] = y - arg3;
    }
    vy3[0] = y;
    if (i == 0) {
      vy3[0] = yInner;
    }
    z = -func_8015477C(FIELD_REF(s32, D_1F800044, 0x34),
                       FIELD_REF(s32, D_1F800044, 0x38)) / 2;
    vy3[-11] = vy3[-7] = (u16)(z - (arg1 * 2 + 0xC0));
    z = -func_8015477C(FIELD_REF(s32, D_1F800044, 0x34),
                       FIELD_REF(s32, D_1F800044, 0x38)) / 2;
    vy3[-3] = vy3[1] = (u16)(z - 0xC0);
    RotTransPers4((SVECTOR*)(vy3 - 0xD), (SVECTOR*)(vy3 - 9),
                  (SVECTOR*)(vy3 - 5), (SVECTOR*)(vy3 - 1), (long*)&prim->x0,
                  (long*)&prim->x1, (long*)&prim->x2, (long*)&prim->x3, &depth,
                  &flag);
    func_80155560(tex[i], (void*)prim, 1);
    func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                  FIELD_REF(s32, D_1F800044, 0x38), 1, 0x28);
  }
}
