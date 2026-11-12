#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Texture-state filler of the main executable: it writes the texture words of
 * the primitive it is handed from the packed state word in its first
 * argument. */
/* @source 0x80155560 */
extern void func_80155560(u32 arg0, void* arg1, s32 arg2);

/* Grid-cell primitive linker of the main executable: it queues the primitive
 * at the shared cursor under the given 16.16 grid cell and advances the
 * cursor by the given size class. */
/* @source 0x80155A08 */
extern void func_80155A08(s32 column, s32 row, s32 mode, s32 size);

/* The scratchpad deck of four 8-byte vectors at 0x1F800014 that
 * RotTransPers4 projects and this body fills. */
/* @source 0x1F800014 @kind data */
extern SVECTOR D_1F800014[];

/* The scratchpad cell holding the work-record pointer every handler of this
 * overlay works through. */
/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;

/* @source 0x801F54B8
 * @behavior Draws the overlay panel quad of the world02/area077 index 14
 *           handlers, which every state step of the target ends with. Reads
 *           the scratchpad work-record pointer cell 0x1F800044 and shifts its
 *           word 0x34 and word 0x38 right by nine bits, which centre the quad
 *           in the x and y lanes, and takes its unsigned halfword 0x3E as the
 *           depth centre. Stamps the shared primitive cursor g_PrimCursor as a
 *           POLY_FT4 with the shade texture off, then fills the four
 *           scratchpad vectors at 0x1F800014 with the corners: the x lane is
 *           the shifted word 0x34 less 0x4040 for the first and third vectors
 *           and less 0x3F40 for the second and fourth, the y lane is the
 *           shifted word 0x38 less 0x4030 for the first two vectors and less
 *           0x3FD0 for the last two, so the quad spans 0x100 by 0x60 around
 *           that corner pair, and the z lane is the depth halfword less 0x30
 *           for the first two vectors and plus 0x30 for the last two.
 *           RotTransPers4 projects the four corners into the primitive's four
 *           8-byte-spaced vertex words at +0x08, +0x10, +0x18 and +0x20, its
 *           projection depth and GTE flag landing in two frame locals. The
 *           primitive then takes the packed texture state word 0x21800133
 *           through the shared filler at 0x80155560 with mode 1 and is queued
 *           by the shared linker at 0x80155A08 at the fixed grid cell 0x5D by
 *           0x34 with mode 0 and size 0x28. Takes no arguments and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTexturedQuadAtWorkCursorPairAndDepth3eLinkedAtGridCell5d34World02Area07714_801F54B8(
    void) {
  u8* work;
  POLY_FT4* prim;
  s32 x;
  s32 y;
  long depth;
  long flag;

  prim = (POLY_FT4*)g_PrimCursor;
  work = workRecordCursor1F800044;
  x = *(s32*)(work + 0x34) >> 9;
  y = *(s32*)(work + 0x38) >> 9;

  SetPolyFT4(prim);
  SetShadeTex(prim, 0);

  D_1F800014[0].vx = D_1F800014[2].vx = x - 0x4040;
  D_1F800014[1].vx = D_1F800014[3].vx = x - 0x3F40;
  SPAD_REF(s16, 0x16u) = SPAD_REF(s16, 0x1Eu) = y - 0x4030;
  SPAD_REF(s16, 0x26u) = SPAD_REF(s16, 0x2Eu) = y - 0x3FD0;
  SPAD_REF(s16, 0x18u) = SPAD_REF(s16, 0x20u) =
      FIELD_REF(u16, workRecordCursor1F800044, 0x3Eu) - 0x30;
  SPAD_REF(s16, 0x28u) = SPAD_REF(s16, 0x30u) =
      FIELD_REF(u16, workRecordCursor1F800044, 0x3Eu) + 0x30;

  RotTransPers4(D_1F800014, D_1F800014 + 1, D_1F800014 + 2,
                D_1F800014 + 3, (long*)((u8*)prim + 8),
                (long*)((u8*)prim + 0x10), (long*)((u8*)prim + 0x18),
                (long*)((u8*)prim + 0x20), &depth, &flag);
  func_80155560(0x21800133, (void*)prim, 1);
  func_80155A08(0x5D0000, 0x340000, 0, 0x28);
}
