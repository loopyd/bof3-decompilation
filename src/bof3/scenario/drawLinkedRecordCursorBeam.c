#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F8D90
 * @behavior Draws one textured cursor beam between the cursors of the two
 * 152-byte work-area records this overlay keeps at 0x80147A58, both selected
 * through the shared scratchpad work object published at 0x1F800044: the work
 * object's word at 0x0C indexes the first record, whose cursor words at
 * 0x34/0x38/0x3C are copied into the work object at the same offsets, and the
 * work object's word at 0x18 indexes the second. The first record's cursor pair
 * is linked through the shared grid-cell primitive linker func_80155A08 with
 * kind 0x11 and size class 0x0C after an 8-byte (x, y, w, h) rect of
 * (0, 240, 16, 16) is written at the primitive cursor g_PrimCursor, the cursor
 * advanced by 8 and the 12-byte texture-window packet that follows the rect
 * built from it by func_8017C1A8. The current primitive is then seeded as a
 * POLY_FT4 and each record's fixed-point origin, resolved through
 * func_801AC1DC(origin, work byte 0x10) for the first record and
 * (origin, work byte 0x1C) for the second, is turned into a position vector
 * whose x and y are ((origin + record cursor word 0x34/0x38) >> 9) - 0x4000 and
 * whose z is -(origin z + record halfword 0x3E) / 2; RotTransPers projects the
 * first record's vector into the quad's first vertex pair and the second
 * record's into its second pair, each projection depth replacing the work
 * object's word at 0x60, and the two pairs are widened by 8 screen units
 * (x1 = x0 + 8, y1 = y0, x0 = x0 - 8, then x3 = x2 + 8, y3 = y2, x2 = x2 - 8).
 * The work object is restored at 0x1F800044, the quad takes clut 176/483,
 * tpage (0, 0, 704, 256), u0 = u2 = 0 and u1 = u3 = 16, all four v coordinates
 * ((u32)D_80143E6C >> 2) & 0x15 with v2 and v3 each adding the absolute
 * difference of the quad's signed halfwords at 0x1A and 0x0A, r0 = g0 = b0 =
 * 0x80 and a cleared shade texture; the work object's cursor pair is then
 * linked again with size class 0x28, a second (0, 0, 256, 256) rect reserves
 * one more texture-window packet through func_8017C1A8, and a final 0x0C-class
 * link closes the frame. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawLinkedRecordCursorBeam(void) {
  u8*              work;
  Scena00WorkCell* work_cell;
  Scena00WorkCell* first;
  Scena00WorkCell* second;
  u8*              prim;
  POLY_FT4*        poly;
  s32              origin[3];
  SVECTOR          projection;
  long             depth;
  long             flag;

  work = D_1F800044;
  prim = g_PrimCursor;
  work_cell = (Scena00WorkCell*)work;
  first = (Scena00WorkCell*)(D_80147A58 + *(s32*)(work + 0x0C) * 152);

  work_cell->cursor_34.coord_x_00 = first->cursor_34.coord_x_00;
  work_cell->cursor_34.coord_y_04 = first->cursor_34.coord_y_04;
  work_cell->cursor_34.scale_08.word = first->cursor_34.scale_08.word;
  g_PrimCursor = prim + 8;
  D_1F800044 = (u8*)first;
  *(s16*)(prim + 0) = 0;
  *(s16*)(prim + 2) = 240;
  *(s16*)(prim + 4) = 16;
  *(s16*)(prim + 6) = 16;
  func_8017C1A8((DR_TWIN*)(prim + 8), (RECT*)prim);
  func_80155A08(*(s32*)(D_1F800044 + 0x34), *(s32*)(D_1F800044 + 0x38), 0x11,
                0x0C);

  poly = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(poly);

  func_801AC1DC(origin, work[0x10]);

  projection.vx = ((origin[0] + *(s32*)(D_1F800044 + 0x34)) >> 9) - 0x4000;
  projection.vy = ((origin[1] + *(s32*)(D_1F800044 + 0x38)) >> 9) - 0x4000;
  projection.vz = -(origin[2] + *(s16*)(D_1F800044 + 0x3E)) / 2;

  *(s32*)(work + 0x60) =
      RotTransPers(&projection, (long*)((u8*)poly + 8), &depth, &flag);

  poly->x1 = poly->x0 + 8;
  poly->x0 = poly->x0 - 8;
  poly->y1 = poly->y0;

  second = (Scena00WorkCell*)(D_80147A58 + *(s32*)(work + 0x18) * 152);
  D_1F800044 = (u8*)second;
  func_801AC1DC(origin, work[0x1C]);

  projection.vx = ((origin[0] + *(s32*)(D_1F800044 + 0x34)) >> 9) - 0x4000;
  projection.vy = ((origin[1] + *(s32*)(D_1F800044 + 0x38)) >> 9) - 0x4000;
  projection.vz = -(origin[2] + *(s16*)(D_1F800044 + 0x3E)) / 2;

  *(s32*)(work + 0x60) =
      RotTransPers(&projection, (long*)((u8*)poly + 0x18), &depth, &flag);

  poly->x3 = poly->x2 + 8;
  poly->x2 = poly->x2 - 8;
  poly->y3 = poly->y2;
  D_1F800044 = work;

  poly->clut = GetClut(176, 483);
  poly->tpage = GetTPage(0, 0, 704, 256);

  poly->u0 = 0;
  poly->v0 = ((u32)D_80143E6C >> 2) & 0x15;
  poly->u1 = 16;
  poly->v1 = ((u32)D_80143E6C >> 2) & 0x15;
  poly->u2 = 0;
  poly->v2 = (((u32)D_80143E6C >> 2) & 0x15) + abs(poly->y2 - poly->y0);
  poly->u3 = 16;
  poly->v3 = (((u32)D_80143E6C >> 2) & 0x15) + abs(poly->y2 - poly->y0);

  poly->r0 = 0x80;
  poly->g0 = 0x80;
  poly->b0 = 0x80;

  SetShadeTex(poly, 1);

  func_80155A08(*(s32*)(D_1F800044 + 0x34), *(s32*)(D_1F800044 + 0x38), 0x11,
                0x28);

  prim = g_PrimCursor;
  g_PrimCursor = prim + 8;
  *(s16*)(prim + 0) = 0;
  *(s16*)(prim + 2) = 0;
  *(s16*)(prim + 4) = 256;
  *(s16*)(prim + 6) = 256;
  func_8017C1A8((DR_TWIN*)(prim + 8), (RECT*)prim);

  func_80155A08(*(s32*)(D_1F800044 + 0x34), *(s32*)(D_1F800044 + 0x38), 0x11,
                0x0C);
}
