#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F6E48
 * @behavior Adopts the 152-byte work-area record selected by the shared
 * scratchpad work object (pointer published at 0x1F800044) and draws that
 * record's cursor beam: the work object's word at 0x18 indexes the record at
 * 0x80147A58 + index * 152, the record's cursor words at 0x34/0x38/0x3C are
 * copied into the work object at the same offsets, and the record pointer
 * replaces the work object at 0x1F800044. The record's cursor pair is linked
 * through the shared grid-cell primitive linker with kind 0x11 and size class
 * 0x0C, after reserving an 8-byte (x, y, w, h) rect of (0, 240, 16, 16) at the
 * primitive cursor and building the 12-byte texture-window packet that follows
 * it from that rect. The record's fixed-point origin is resolved from the work
 * object's byte at 0x1C and turned into a position vector whose x/y are
 * ((origin + record cursor word) >> 9) - 0x4000 and whose z is
 * -(origin z + record halfword 0x3E) / 2; projecting it writes the packed
 * screen pair into the flat textured quad at the primitive cursor, stores the
 * work object's word at 0x60, and leaves x0 = x2 = projected x - 8, x1 = x3 =
 * projected x + 8, y0 = y1 = 0 and y2 = y3 = projected y. The quad keeps
 * clut 176/483, tpage (0, 0, 704, 256), u0 = u2 = 0, u1 = u3 = 16, v0 = v1 =
 * ((u32)D_80143E6C >> 2) & 0x15 and v2 = v3 = that value plus the quad's low byte
 * at 0x1A, r0 = g0 = b0 = 0x80 and a cleared shade texture. The work object is
 * restored at 0x1F800044, linked again with size class 0x28, a second
 * (0, 0, 256, 256) rect reserves one more texture-window packet, and a final
 * 0x0C-class link closes the frame. Takes no arguments and returns nothing.
 * Decisive clean-C shape: the two cursor-word copies go through the reviewed
 * Scena00WorkCell/Scena00Cursor fields (struct-member addressing) instead of
 * s32 pointer casts; that is what reproduces the original load/store schedule.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawAdoptedRecordCursorBeam(void) {
  u8*              work;
  Scena00WorkCell* work_cell;
  Scena00WorkCell* adopted;
  u8*              prim;
  POLY_FT4*        poly;
  s32              origin[3];
  SVECTOR          projection;
  long             depth;
  long             flag;
  u16              x;
  u16              x2;
  u16              y;

  work = D_1F800044;
  prim = g_PrimCursor;
  work_cell = (Scena00WorkCell*)work;
  adopted = (Scena00WorkCell*)(D_80147A58 + *(s32*)(work + 0x18) * 152);

  work_cell->cursor_34.coord_x_00 = adopted->cursor_34.coord_x_00;
  work_cell->cursor_34.coord_y_04 = adopted->cursor_34.coord_y_04;
  work_cell->cursor_34.scale_08.word = adopted->cursor_34.scale_08.word;
  g_PrimCursor = prim + 8;
  D_1F800044 = (u8*)adopted;
  *(s16*)(prim + 0) = 0;
  *(s16*)(prim + 2) = 240;
  *(s16*)(prim + 4) = 16;
  *(s16*)(prim + 6) = 16;
  func_8017C1A8((DR_TWIN*)(prim + 8), (RECT*)prim);
  func_80155A08(*(s32*)(D_1F800044 + 0x34), *(s32*)(D_1F800044 + 0x38),
                0x11, 0x0C);

  poly = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(poly);

  func_801AC1DC(origin, work[0x1C]);

  projection.vx = (((origin[0] + *(s32*)(D_1F800044 + 0x34)) >> 9) - 0x4000);
  projection.vy = (((origin[1] + *(s32*)(D_1F800044 + 0x38)) >> 9) - 0x4000);
  projection.vz = -(origin[2] + *(s16*)(D_1F800044 + 0x3E)) / 2;

  *(s32*)(work + 0x60) =
      RotTransPers(&projection, (long*)((u8*)poly + 8), &depth, &flag);

  y = poly->y0;
  x = poly->x0;
  poly->y1 = 0;
  poly->y0 = 0;
  D_1F800044 = work;
  x2 = poly->x0;
  poly->x1 = x + 8;
  poly->x3 = x + 8;
  poly->x0 = x2 - 8;
  poly->x2 = x2 - 8;
  poly->y3 = y;
  poly->y2 = y;
  poly->clut = GetClut(176, 483);
  poly->tpage = GetTPage(0, 0, 704, 256);

  poly->u0 = 0;
  poly->v0 = ((u32)D_80143E6C >> 2) & 0x15;
  poly->u1 = 16;
  poly->v1 = ((u32)D_80143E6C >> 2) & 0x15;
  poly->u2 = 0;
  poly->v2 = (((u32)D_80143E6C >> 2) & 0x15) + (u8)poly->y2;
  poly->u3 = 16;
  poly->v3 = (((u32)D_80143E6C >> 2) & 0x15) + (u8)poly->y2;

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
