#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell this overlay publishes and every handler
 * of the target reads. The original materialises the cell with `lui` plus an
 * offset load for every access, so the body spells the cell itself where the
 * value is needed once and keeps one pointer local only where the corner loop
 * needs it across the projection call.
 * @source 0x1F800044 @kind bss
 */
extern u8* D_1F800044;

/* The four scratchpad vectors at 0x1F800014 that RotTransPers4 projects into
 * the primitive's four corner pairs. The corner loop writes their halfword
 * lanes through constant-address scratchpad lvalues, so this base only feeds
 * the four vector arguments.
 * @source 0x1F800014 @kind data
 */
extern SVECTOR D_1F800014[];

/* Shared primitive cursor of the main executable, which every quad of the run
 * is built at. Splat binds the address through this target's generated
 * undefined_syms_auto.txt.
 * @source 0x8014598C @kind bss
 */
extern u8* g_PrimCursor;

/* Shade table of this overlay: two signed bytes per record byte 0, the first
 * for the work record whose byte 2 is 2 and the second for every other value.
 * A negative entry selects the alternate append path.
 * @source 0x801F4240 @kind data
 */
extern s8 D_801F4240[];

/* First record index of the run: one byte per doubled work halfword 0x36. */
extern u8 D_801F4250[]; /* @source 0x801F4250 @kind data */

/* One past the last record index of the run: the same doubling of work
 * halfword 0x36, re-read for every loop test. */
extern u8 D_801F4251[]; /* @source 0x801F4251 @kind data */

/* Corner offset rows: twelve signed bytes per record byte 1, three per corner
 * (x, y and z), each in eighth-of-a-cell units. Addressed as three byte symbols
 * one apart so every corner access re-materialises its own base, as the
 * original does. */
extern s8 D_801F4258[]; /* @source 0x801F4258 @kind data */
extern s8 D_801F4259[]; /* @source 0x801F4259 @kind data */
extern s8 D_801F425A[]; /* @source 0x801F425A @kind data */

/* The run's 8-byte records: word 0 packs the shade row in byte 0, the corner
 * offset row in byte 1, the row cell in byte 2 and the column cell in byte 3;
 * word 1 is the record's texture-state word. Both words are addressed from
 * their own symbol, so each access re-materialises the address the way the
 * original does. */
extern u8 D_801F4294[]; /* @source 0x801F4294 @kind data */
extern u8 D_801F4298[]; /* @source 0x801F4298 @kind data */

/* Primitive linker of the main executable and the alternate append service.
 * The linker moves the record cell pair through the shared cell-size pair,
 * links the packet at the cursor with the given kind and advances the cursor by
 * the given size; the append service takes the same size with kind 3. */
extern void func_80155A08(s32 arg0, s32 arg1, s32 arg2,
                          s32 arg3); /* @source 0x80155A08 */
extern void func_8014E5A0(s32 arg0, s32 arg1); /* @source 0x8014E5A0 */

/* Texture-state filler of the main executable: it writes the packed texture
 * page, texture window and colour lookup word into the packet at the cursor and
 * starts its texture-state run. */
extern void func_80155560(u32 arg0, void* arg1, s32 arg2); /* @source 0x80155560 */

/* @source 0x801F3274
 * @behavior Draws one run of textured quads for the work record published at
 *           the scratchpad pointer cell 0x1F800044: the record's signed
 *           halfword 0x36, doubled, indexes the two byte tables at 0x801F4250
 *           and 0x801F4251, which give the first record index of the run and
 *           the one past its last. Nothing is drawn when the first is not below
 *           the last, and the bound is re-read after every record so a handler
 *           that shrinks it ends the run early.
 *           Each record is an 8-byte entry of the table at 0x801F4294. Its
 *           word 0 packs four bytes: the shade row at 0, the corner offset row
 *           at 1, the row cell at 2 and the column cell at 3. The two cells are
 *           biased into the shared cell grid, ((cell - 0x80) << 7) + 0x40, and
 *           kept in the high halfword of a 16.16 corner pair whose low halfword
 *           the original preserves across the step, which is the `andi 0xFFFF`
 *           the pair takes at the top of every iteration.
 *           The four corners then take that column and row minus the corner
 *           offset row's own x and y bytes shifted left seven, and the work
 *           record's halfword 0x3E plus the row's z byte shifted left seven as
 *           depth, all written into the four scratchpad vectors at 0x1F800014
 *           in 8-byte steps. RotTransPers4, declared by the PsyQ header this
 *           target includes, projects them into the four corner pairs of the
 *           POLY_FT4 stamped at the shared primitive cursor with flat shading,
 *           and func_80155560 fills its texture state from the record's own
 *           word 1 with texture-state argument 1.
 *           The record byte 0, doubled and stepped by one when work byte 2 is
 *           2, indexes the signed shade table at 0x801F4240; a negative entry
 *           appends the 0x28-byte primitive through func_8014E5A0 with kind 3,
 *           and every other entry links it through func_80155A08 with that
 *           byte as the kind and the record's column and row cells as the
 *           16.16 position pair. Takes no arguments and returns nothing: the
 *           four handlers 0x801F2C5C, 0x801F2D18, 0x801F2DA0 and 0x801F2E14
 *           call it with the argument registers untouched.
 * @status partial
 * @match 66.67
 * @residual 124 of 186 instructions match at 740 -> 744 bytes. The frame
 *           matches; the first mismatch is +0x0040, where the run index uses
 *           s2 instead of s4. Register allocation and scheduling still differ.
 */
void drawRecordQuadRunSlicedByWorkHalfword36World02Area11213_801F3274(void) {
  SVECTOR* vec;
  POLY_FT4* prim;
  u8* rec;
  u8* work;
  s32 index;
  s32 off;
  s32 x;
  s32 y;
  s32 cx;
  s32 cy;
  s32 i;
  s32 rowOff;
  s32 voff;
  s32 ox;
  s32 shade;
  long p;
  long flag;

  index = D_801F4250[*(s16*)(D_1F800044 + 0x36) * 2];
  if (index < D_801F4251[*(s16*)(D_1F800044 + 0x36) * 2]) {
    vec = D_1F800014;
    rec = D_801F4294 + index * 8;
    off = index * 8;
record_loop:
    {
      x = x & 0xFFFF;
      prim = (POLY_FT4*)g_PrimCursor;
      y = y & 0xFFFF;
      SetPolyFT4(prim);
      SetShadeTex(prim, 0);
      i = 0;
      rowOff = 0;
      voff = 0;
      work = D_1F800044;
      x = x | ((((*(u32*)rec >> 24) - 0x80) << 7) + 0x40) << 16;
      y = y | (((((*(u32*)rec >> 16) & 0xFF) - 0x80) << 7) + 0x40) << 16;
      cx = x >> 16;
      cy = y >> 16;
corner_loop:
      {
        ox = D_801F4258[rec[1] * 12 + rowOff];
        *(s16*)(0x1F800014 + voff) = cx - (ox << 7);
        ox = D_801F4259[rec[1] * 12 + rowOff];
        *(s16*)(0x1F800016 + voff) = cy - (ox << 7);
        ox = D_801F425A[rec[1] * 12 + rowOff];
        *(s16*)(0x1F800018 + voff) = *(u16*)(work + 0x3E) + (ox << 7);
        i++;
        rowOff += 3;
        voff += 8;
      }
      if (i < 4) goto corner_loop;
      RotTransPers4(vec, vec + 1, vec + 2, vec + 3, (long*)&prim->x0,
                    (long*)&prim->x1, (long*)&prim->x2, (long*)&prim->x3, &p,
                    &flag);
      func_80155560(*(u32*)(D_801F4298 + off), (void*)prim, 1);
      shade = D_801F4240[(*(u32*)rec & 0xFF) * 2 + (D_1F800044[2] == 2)];
      if (shade < 0) {
        func_8014E5A0(3, 0x28);
      } else {
        func_80155A08((*(u32*)rec >> 8) & 0xFF0000, *(u32*)rec & 0xFF0000, shade,
                      0x28);
      }
      index++;
      rec += 8;
      off += 8;
    }
    if (index < D_801F4251[*(s16*)(D_1F800044 + 0x36) * 2]) goto record_loop;
  }
}
