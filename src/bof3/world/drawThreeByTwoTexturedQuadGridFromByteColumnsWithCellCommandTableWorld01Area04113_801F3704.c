#include "bof3/bof3.h"
#include "gpu/prim.h"

/* The four scratchpad vectors at 0x1F800014 that RotTransPers4 projects. The
 * body keeps their base in a saved register and writes the halfword lanes
 * through it. */
extern SVECTOR D_1F800014[]; /* @source 0x1F800014 @kind data */

/* The six-cell coordinate table of the grid: one byte pair per cell, the first
 * column byte at 0x801F4364 and the second at 0x801F4365, both walked with a
 * stride of two. The map names the second column byte, so the first column is
 * that pointer minus one. */
extern u8 D_801F4365[]; /* @source 0x801F4365 @kind data */

/* The six-byte per-cell command table read with the loop index. */
extern u8 D_801F4370[]; /* @source 0x801F4370 @kind data */

void func_80155560(u32 arg0, void* arg1, s32 arg2);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F3704
 * @behavior Draws the overlay's three-by-two grid of textured quads over six
 *           steps of one loop. Each step stamps a POLY_FT4 at the shared
 *           primitive cursor g_PrimCursor with SetPolyFT4 and SetShadeTex off,
 *           fills the four scratchpad vectors at 0x1F800014 with the current
 *           cell's corners and projects them into the primitive's x0 to x3
 *           words with RotTransPers4. The corners come from the cell's byte
 *           pair: the first column byte times 128 minus 0x3FC0 is both the x
 *           lane of the first two vectors and, minus 0x4040, the x lane of the
 *           last two, so the two columns are 0x80 apart; the second column byte
 *           gives the y lanes the same way, and every z lane takes -0x88. The
 *           step then hands the primitive to 0x80155560 with the word
 *           0xBD500000 carrying the step's byte out of the six-byte table at
 *           0x801F4370 and mode 1, and queues it through 0x80155A08 with both
 *           column bytes shifted up sixteen bits, a zero selector and size
 *           0x28. Both column pointers advance by two per step, so the table
 *           walks 0x2A/0x2B/0x2C against 0x1D for the first row and the same
 *           three columns against 0x1E for the second, and the loop ends after
 *           six steps. Takes no arguments and returns nothing; the meaning of
 *           the per-cell command byte stays offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawThreeByTwoTexturedQuadGridFromByteColumnsWithCellCommandTableWorld01Area04113_801F3704(
    void) {
  SVECTOR* vec;
  u8* px;
  u8* py;
  POLY_FT4* prim;
  s32 i;
  s16 depth;
  long p;
  long flag;

  i = 0;
  vec = D_1F800014;
  depth = -0x88;
  py = D_801F4365;
  px = py - 1;
  for (; i < 6; i++, py += 2, px += 2) {
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);

    vec[0].vx = vec[1].vx = *px * 128 - 0x3FC0;
    vec[2].vx = vec[3].vx = *px * 128 - 0x4040;
    vec[0].vy = vec[2].vy = *py * 128 - 0x3FC0;
    vec[1].vy = vec[3].vy = *py * 128 - 0x4040;
    vec[0].vz = vec[1].vz = vec[2].vz = vec[3].vz = depth;

    RotTransPers4(vec, vec + 1, vec + 2, vec + 3, (long*)&prim->x0,
                  (long*)&prim->x1, (long*)&prim->x2, (long*)&prim->x3, &p,
                  &flag);
    func_80155560(D_801F4370[i] | 0xBD500000, (void*)prim, 1);
    func_80155A08(*px << 16, *py << 16, 0, 0x28);
  }
}
