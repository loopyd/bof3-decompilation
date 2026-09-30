#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F8BCC
 * @behavior Builds one screen-space flat textured quad for the scratchpad work
 * object published at 0x1F800044 and appends it at the shared primitive
 * cursor: takes the POLY_FT4 at g_PrimCursor, lays every corner at the
 * object's cursor halfwords at offsets 0x2E (x) and 0x30 (y) minus half the
 * object's transition byte at offset 0x06, adds that whole byte back on the
 * right and bottom corners (x1 and x3 use x + 1, y2 and y3 use y + 1), fills
 * the texture window with u = 0xE0/0xFF and v = 0x30/0x50, takes its clut from
 * GetClut(160, 483) and its tpage from GetTPage(0, 1, 704, 256), copies the
 * object's three tint bytes at 0x5D/0x5E/0x5F into r0/g0/b0, clears the shade
 * texture, turns semi-transparency on, and appends the 0x28-byte primitive
 * through func_8014E5A0 with index 3 once the shared effect bank D_8014832E is
 * armed. Takes no arguments and returns nothing; the address is a direct call
 * target of the lifted handlers func_801F878C, func_801F882C, func_801F8930
 * and func_801F8AB8.
 * @status exact
 * @match 100.00
 * @residual none
 * The leading transition-byte read spells the published pointer cell
 * D_1F800044 directly (`(D_1F800044[6] >> 1) & 0xFF`), which keeps that load in
 * $v0 hoisted above the prologue; binding the same access to the `u8* work`
 * local used for the corner block allocated the load to $a0 instead
 * (102/105, first=+0x0000) because the local's range crosses the SetPolyFT4
 * call. The corner block keeps the post-call `u8* work` local: it is
 * rematerialised once into $a0 and reused for all eight halfword stores,
 * while the three tint bytes below spell the cell itself so each byte store
 * forces its own fresh `lui/lw` exactly as the original does.
 */
void func_801F8BCC(void) {
  POLY_FT4* prim;
  u8* work;
  u8 offset;

  offset = (D_1F800044[6] >> 1) & 0xFF;
  prim = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(prim);
  work = D_1F800044;

  prim->x0 = *(u16*)(work + 0x2E) - offset;
  prim->y0 = *(u16*)(work + 0x30) - offset;
  prim->x1 = *(u16*)(work + 0x2E) - offset + work[6];
  prim->y1 = *(u16*)(work + 0x30) - offset;
  prim->x2 = *(u16*)(work + 0x2E) - offset;
  prim->y2 = *(u16*)(work + 0x30) - offset + work[6];
  prim->x3 = *(u16*)(work + 0x2E) - offset + work[6];
  prim->y3 = *(u16*)(work + 0x30) - offset + work[6];

  prim->u0 = 0xE0;
  prim->v0 = 0x30;
  prim->u1 = 0xFF;
  prim->v1 = 0x30;
  prim->u2 = 0xE0;
  prim->v2 = 0x50;
  prim->u3 = 0xFF;
  prim->v3 = 0x50;

  prim->r0 = D_1F800044[0x5D];
  prim->g0 = D_1F800044[0x5E];
  prim->b0 = D_1F800044[0x5F];
  prim->clut = GetClut(160, 483);
  prim->tpage = GetTPage(0, 1, 704, 256);
  SetShadeTex(prim, 0);
  SetSemiTrans(prim, 1);

  if (D_8014832E != 0) {
    func_8014E5A0(3, 0x28);
  }
}
