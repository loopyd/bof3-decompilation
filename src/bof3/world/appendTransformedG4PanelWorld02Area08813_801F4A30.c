#include "bof3/bof3.h"

/* Scratchpad vertex block at 0x1F800014: the four eight-byte SVECTORs seeded
 * below and then projected. Only the first three halfwords of each record are
 * written, the fourth is the packing pad the projector ignores. The block is
 * reached through one pointer local, so its address stays in a register and the
 * first halfword of every record is stored through that base plus its
 * displacement rather than through a literal address of its own.
 * @source 0x1F800014 @kind data
 */
extern SVECTOR D_1F800014;

/* Shared world-angle halfword triplet the rotation matrix is built from.
 * @source 0x801492D8 @kind data
 */
extern SVECTOR D_801492D8;

/* Primitive cursor the panel is written into.
 * @source 0x8014598C @kind bss
 */
extern u8* g_PrimCursor;

/* Ordering-table append helper that links the primitive cursor into the given
 * table slot for the given byte size. */
void func_8014E5A0(u32 ot_index, u32 primitive_size);

/* @source 0x801F4A30
 * @behavior Pushes the GTE matrix stack and composes a rotation-only matrix
 *           from the shared angle triplet 0x801492D8 with the translation field
 *           cleared, installing it through SetRotMatrix and SetTransMatrix.
 *           Seeds the four eight-byte scratchpad vertices at 0x1F800014 with the
 *           cross offsets (-10,0,0), (0,-4,0), (0,4,0) and (10,0,0), opens the
 *           four-corner Gouraud quad on the primitive cursor cell 0x8014598C,
 *           and projects those vertices into its corners with RotTransPers4,
 *           homing the depth and flag words in the frame. Paints corner 0
 *           (0xFF,0,0), corners 1 and 2 (0x80,0,0x80) and corner 3 (0,0,0xFF),
 *           then moves every projected coordinate by minus 0x9E horizontally
 *           and 0x76 vertically and adds the two signed arguments, so the panel
 *           lands at the point they name. Queues the 0x24-byte primitive through
 *           the shared append helper under ordering-table index 1 and pops the
 *           matrix stack. Takes a signed screen anchor and returns nothing.
 * @status partial
 * @match 96.03
 * @residual 121 of 126 original instructions match at the original 504-byte
 *           size. The five that differ sit in one scheduling cluster between
 *           the projection call and the colour stores: the original issues its
 *           third and fourth projection arguments as a2 then a3, this source
 *           issues a3 then a2; the original materialises the append size 0x24
 *           (a1) directly after the append index and the 0xFF colour into a2
 *           before the corner 0 y load, while this source keeps 0xFF in v0,
 *           groups the corner 3 blue store beside the corner 0 red store, and
 *           places both the y load and the 0x24 constant after that group.
 *           Every other instruction, the frame, both matrix pairs, the twelve
 *           scratchpad seeds, the projection, all eight corner coordinates and
 *           the tail match.
 */
void appendTransformedG4PanelWorld02Area08813_801F4A30(s16 arg0, s16 arg1) {
  MATRIX matrix;
  long depth;
  long flag;
  POLY_G4* prim;
  SVECTOR* vertices;
  SVECTOR* seed;

  PushMatrix();
  RotMatrix(&D_801492D8, &matrix);
  matrix.t[0] = 0;
  matrix.t[1] = 0;
  matrix.t[2] = 0;
  SetRotMatrix(&matrix);
  SetTransMatrix(&matrix);

  prim = (POLY_G4*)g_PrimCursor;
  vertices = &D_1F800014;

  /* One record per cross offset: the x halfword is stored through the
   * advancing base and the other two through their own fixed addresses. */
  seed = &D_1F800014;
  seed->vx = -10;
  SPAD_REF(SVECTOR, 0x14u).vy = 0;
  SPAD_REF(SVECTOR, 0x14u).vz = 0;
  seed++;
  seed->vx = 0;
  SPAD_REF(SVECTOR, 0x1cu).vy = -4;
  SPAD_REF(SVECTOR, 0x1cu).vz = 0;
  seed++;
  seed->vx = 0;
  SPAD_REF(SVECTOR, 0x24u).vy = 4;
  SPAD_REF(SVECTOR, 0x24u).vz = 0;
  seed++;
  seed->vx = 10;
  SPAD_REF(SVECTOR, 0x2cu).vy = 0;
  SPAD_REF(SVECTOR, 0x2cu).vz = 0;

  SetPolyG4(prim);
  RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                (long*)&prim->x3, &depth, &flag);

  prim->r1 = 0x80;
  prim->b1 = 0x80;
  prim->r2 = 0x80;
  prim->b2 = 0x80;
  prim->r0 = 0xff;
  prim->g0 = 0;
  prim->b0 = 0;
  prim->g1 = 0;
  prim->g2 = 0;
  prim->r3 = 0;
  prim->g3 = 0;
  prim->b3 = 0xff;

  prim->x0 = (s16)(prim->x0 - 0x9e) + arg0;
  prim->y0 = (s16)(prim->y0 - 0x76) + arg1;
  prim->x1 = (s16)(prim->x1 - 0x9e) + arg0;
  prim->y1 = (s16)(prim->y1 - 0x76) + arg1;
  prim->x2 = (s16)(prim->x2 - 0x9e) + arg0;
  prim->y2 = (s16)(prim->y2 - 0x76) + arg1;
  prim->x3 = (s16)(prim->x3 - 0x9e) + arg0;
  prim->y3 = (s16)(prim->y3 - 0x76) + arg1;

  func_8014E5A0(1, 0x24);
  PopMatrix();
}
