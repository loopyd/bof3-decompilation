#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Work record of this overlay, published through the scratchpad pointer cell
 * 0x1F800044. The original re-reads the cell before the entry gate, after
 * PushMatrix and after RotTransPers4, so every block binds it again.
 * @source 0x1F800044 @kind unknown
 */
extern u8* area12114WorkCursor_1F800044;

/* Shared effect bank byte at 0x8014832E. Bits 0x1B let this handler draw.
 * @source 0x8014832E @kind unknown
 */
extern u8 D_8014832E;

/* Shared rotation matrix at 0x801492E8 that orients the projected quad.
 * @source 0x801492E8 @kind unknown
 */
extern MATRIX D_801492E8;

/* The four scratchpad vectors at 0x1F800014 that RotTransPers4 projects. The
 * original keeps their base in a saved register for the four vector arguments
 * and writes the halfword lanes through absolute addresses.
 * @source 0x1F800014 @kind data
 */
extern SVECTOR D_1F800014[];

/* Shared graph-type probe of the main executable. */
/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* Shared primitive linker at 0x80155A08: resolves the (x, y) cell through the
 * shared cell-size pair, links the primitive at g_PrimCursor with the given
 * kind and advances the cursor by the given size. */
/* @source 0x80155A08 */
extern void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F60F4
 * @behavior Byte-1 state handler 4 of the byte-1 handler table 0x801F68A4 for
 * the work record published at the scratchpad pointer cell 0x1F800044.
 * It adds 0x10 to work halfword 0x3E and stores the truncated halfword, then
 * writes 3 to state byte 0x01 once that stored signed halfword reaches 0x281.
 * The body returns unless any of bits 0x1B of the shared effect bank byte
 * 0x8014832E is set. It then pushes the shared matrix, turns the work words
 * 0x34 and 0x38 into a local point by shifting each right nine and biasing it
 * by -0x4000, takes the negated half of work halfword 0x3E as that point z,
 * rotates the point into the pushed matrix at 0x801492E8 with a zero
 * rotation and installs that matrix for the GTE. It fills the four scratchpad
 * vectors at 0x1F800014 as a diamond of halfsize ((signed work halfword 0x3E
 * shifted right seven) times ten plus 0x10) lying in the xy plane, stamps
 * POLY_FT4 at the primitive cursor, sets semi-transparency with argument 1,
 * and selects the texture page 0xAB when the graph type is 1 or 2 and 0x3B
 * otherwise. The quad takes texture columns 0xE0 to 0xFE and rows 0x30 to 0x4E
 * on clut 0x78CA, is flat 0x80 grey, and is projected into the primitive x0 to
 * x3 slots by RotTransPers4. Finally it links the primitive at the cell of the
 * work words 0x34 and 0x38 with kind 1 at 0x28 bytes and pops the matrix.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHalfword3eBy10AndSetState3At0x281ThenDrawRotatedTexturedDiamondWorld03Area12114_801F60F4(
    void) {
  u8*       work;
  POLY_FT4* prim;
  SVECTOR   src;
  SVECTOR   rotation;
  MATRIX    matrix;
  long      flag;
  long      p;
  long      projected;
  s32       tpage;
  s32       half;

  work = area12114WorkCursor_1F800044;
  FIELD_REF(u16, work, 0x3E) = (u16)(FIELD_REF(u16, work, 0x3E) + 0x10);
  if ((s16)FIELD_REF(u16, work, 0x3E) >= 0x281) {
    work[1] = 3;
  }
  if ((D_8014832E & 0x1B) == 0) {
    return;
  }

  PushMatrix();
  work = area12114WorkCursor_1F800044;
  rotation.vx = 0;
  rotation.vy = 0;
  rotation.vz = 0;
  src.vx = (FIELD_REF(s32, work, 0x34) >> 9) - 0x4000;
  src.vy = (FIELD_REF(s32, work, 0x38) >> 9) - 0x4000;
  src.vz = -(s16)FIELD_REF(u16, work, 0x3E) / 2;
  RotTrans(&src, (VECTOR*)matrix.t, &flag);
  RotMatrix(&rotation, &matrix);
  MulMatrix2(&D_801492E8, &matrix);
  SetRotMatrix(&matrix);
  SetTransMatrix(&matrix);

  p = ((s16)FIELD_REF(u16, area12114WorkCursor_1F800044, 0x3E) >> 7) * 10 + 0x10;
  prim = (POLY_FT4*)g_PrimCursor;
  D_1F800014[0].vx = -(u16)p;
  D_1F800014[0].vy = 0;
  D_1F800014[0].vz = 0;
  D_1F800014[1].vx = 0;
  D_1F800014[1].vy = -(u16)p;
  D_1F800014[1].vz = 0;
  D_1F800014[2].vx = 0;
  D_1F800014[2].vy = (u16)p;
  D_1F800014[2].vz = 0;
  D_1F800014[3].vx = (u16)p;
  D_1F800014[3].vy = 0;
  D_1F800014[3].vz = 0;

  SetPolyFT4(prim);
  SetSemiTrans(prim, 1);
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xAB;
  } else {
    tpage = 0x3B;
  }
  prim->tpage = tpage;
  prim->u0 = 0xE0;
  prim->u1 = 0xFE;
  prim->v0 = 0x30;
  prim->clut = 0x78CA;
  prim->v1 = 0x30;
  prim->u2 = prim->u0;
  prim->u3 = prim->u1;
  half = prim->v0 + 0x1E;
  prim->v2 = half;
  prim->v3 = half;

  RotTransPers4(D_1F800014, D_1F800014 + 1, D_1F800014 + 2, D_1F800014 + 3,
                (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                (long*)&prim->x3, &p, &projected);
  prim->r0 = 0x80;
  prim->g0 = 0x80;
  prim->b0 = 0x80;

  func_80155A08(FIELD_REF(s32, area12114WorkCursor_1F800044, 0x34),
                FIELD_REF(s32, area12114WorkCursor_1F800044, 0x38), 1, 0x28);
  PopMatrix();
}
