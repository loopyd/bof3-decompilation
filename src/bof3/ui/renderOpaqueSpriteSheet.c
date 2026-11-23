#include "bof3/bof3.h"

/* Primitive append helper of the engine: appends the packet the shared cursor
 * holds to the given ordering-table slot with the given byte size. */
extern void func_8014E5A0(s32 ot_index,
                          s32 packet_size); /* @source 0x8014E5A0 */

/* The shared primitive cursor the engine publishes for the overlay emitters. */
extern u8 *g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Eighteen four-byte sprite sheet entries of this overlay, indices 0 through
 * 0x11: the sheet cell u and v, then the sprite width and height. Entries 0
 * through 3 are the 0x20-by-0x18 card face cell with its 8-by-0x18 side
 * strips behind it, 9 and 0xA are the 0x30-by-0x18 cells the card panels show,
 * and the remaining entries are 8-by-8 icon pieces. */
extern u8 spriteSheetEntryTable[]; /* @source 0x801DAC60 @kind table */

/* Representation note, measured against this body's 50 instructions at 200
 * bytes: storing the CLUT word ahead of the cell v store scored 43 of 50 with
 * the first difference a $v0/$v1 swap on the u load and the CLUT store pulled
 * in front of the u store, while writing the two cell bytes ahead of the CLUT
 * store scored 50 of 50 at the original size. */

/* @source 0x801D2EDC
 * @behavior Draws one sprite of the overlay sheet at (x, y) through the shared
 *           primitive cursor. SetSprt initialises the 0x14-byte sprite at the
 *           cursor, the sprite argument selects a four-byte entry of the sheet
 *           table at 0x801DAC60 as its low byte scaled by four, and that entry
 *           gives the cell u and v, the width and the height. The position
 *           comes from the two structural coordinates, the CLUT word is the
 *           fixed 0x7887 of the panel palette and all three colour bytes are
 *           the neutral gray 0x80, so the sprite is left fully opaque. The
 *           sprite is appended at ordering-table slot 1 as 0x14 bytes. Its
 *           0x28-byte frame keeps $s0 through $s3 and $ra across the two calls.
 *           Three callers inside this payload reach the span by jal and fix
 *           all three roles: func_801D2AC4 passes the card-panel cells 9 and
 *           0xA, func_801D2FA4 passes 0 through 8 and func_801D4910 passes the
 *           8-by-8 pieces 0xB through 0x11, each narrowing the two coordinates
 *           to s16. No caller reads a return value, so the span answers none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderOpaqueSpriteSheet(s16 x, s16 y, u8 sprite) {
  s32 offset;
  SPRT *primitive;

  primitive = (SPRT *)g_PrimCursor;
  SetSprt(primitive);
  offset = sprite * 4;
  primitive->x0 = x;
  primitive->y0 = y;
  primitive->w = spriteSheetEntryTable[offset + 2];
  primitive->h = spriteSheetEntryTable[offset + 3];
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  primitive->u0 = spriteSheetEntryTable[offset + 0];
  primitive->v0 = spriteSheetEntryTable[offset + 1];
  primitive->clut = 0x7887;
  func_8014E5A0(1, 0x14);
}
