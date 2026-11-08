#include "bof3/bof3.h"
#include "gpu/prim.h"

extern int rand(void);

/* Work-record pointer cell at 0x801F17B0. The overlay frame loop at 0x801EEC28
 * publishes the current work record into this cell and the dispatcher at
 * 0x801EF37C re-reads it on every access, so the body binds the cell by name
 * and re-reads it at each of the two points the original re-materialises the
 * address: once for the two draw-mode packet words before the random radius
 * seed and once inside the loop for the two triangle packet words. The cell
 * sits in this overlay's own trailing data block, so the reference binds
 * through the target's generated undefined-symbol table. */
extern u8 *D_801F17B0; /* @source 0x801F17B0 @kind data */

/* Palette bytes at 0x801F0158, 0x801F0159 and 0x801F015A: the three channels
 * of the colour triplet the centre vertex of every triangle takes. The
 * original reaches them as three separate indexed loads, each with its own
 * `lui $at` plus `addu $at,$at,index` plus `lbu %lo` sequence, the low halves
 * being 0x0158, 0x0159 and 0x015A, so the body keeps three byte bases and
 * indexes each with the same row offset. The row offset the original emits is
 * twelve times the record byte 0x04 plus three times the record byte 0x03,
 * recomputed in full at each of the three loads. No other span of this overlay
 * references these bytes. */
extern u8 D_801F0158[]; /* @source 0x801F0158 @kind table */
extern u8 D_801F0159[]; /* @source 0x801F0159 @kind table */
extern u8 D_801F015A[]; /* @source 0x801F015A @kind table */

/* Scratchpad halfword at 0x1F800004, the per-triangle radius. The original
 * forms its address once with a `lui` plus `addiu` pair into a general
 * register and stores the seed through that register, unlike every other
 * scratchpad access of this body, which uses the absolute `lui $at` store
 * form: the same register then serves as the loop base, four bytes below, and
 * every loop read of the cells 0x1F800000 through 0x1F80000E is an offset from
 * that derived base. No other span of this overlay references the cell. */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Shared engine primitive stamper, called on the cursor before the body writes
 * the primitive fields. The payload names 0x8017A97C only from this span, so
 * the target map row carries the address. Its prototype is the one the
 * scenario quad emitters bind for the same role. */
void func_8017A97C(void *arg0); /* @source 0x8017A97C */

/* Shared grid-cell primitive linker: it resolves the (x, y) cell through the
 * shared cell-size pair, links the primitive at g_PrimCursor with the given
 * kind and advances the cursor by the given size, which is why the body
 * re-reads g_PrimCursor at the top of every iteration. This body links a
 * 0xC-byte draw-mode packet and then one 0x1C-byte POLY_G3 packet per
 * triangle. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` products below. This body takes the
 * 0x801782FC sample for both vertex x sums and the 0x801783C8 sample for both
 * vertex y sums. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* Naming and shape evidence: the payload holds one byte-identical twin of this
 * 804-byte body, emi/bmagic/magic168/03 at 0x801EFCD8 (one payload comparison
 * over the whole span, equal), whose row is still assembly, so the clone lever
 * has no source to copy and the body was seeded from the original bytes. The
 * nearest exact siblings that named these roles are the drawing steps of the
 * same overlay family: drawFourRotatedSpokesFromPublishedCenterTintedBy
 * WorkByte5BmagicMagic16203_801EF80C for the published centre, the draw-mode
 * disjunction and the 0xC-byte then 0x20-byte linker calls, and
 * drawFourPhasedTrianglesFromOriginTintedByWorkRecordLifetimeBmagicMagic10603
 * _801EF5F4 for the cursor seed order, the POLY_G3 packet pair at record words
 * 0x14 and 0x18 and the 0x1C-byte link size. The name keeps the sibling
 * spelling convention under this target and address: eight triangles rotated
 * by eighth turns around the published centre, the centre vertex tinted by the
 * palette bytes selected by the work record bytes 0x03 and 0x04 scaled by work
 * byte 0x05. */
/* @source 0x801EFCD8
 * @behavior Battle-magic drawing step reached from the draw branch of the
 *           overlay dispatcher at 0x801EF37C when the work record byte 0 has
 *           bit 0 set and the record byte 2 is non-zero; it takes no arguments
 *           and returns nothing. It first picks the draw-mode tpage argument
 *           0xA5 when GetGraphType returns 1 and 0xA5 again when it returns 2,
 *           and 0x35 otherwise, seeds SetDrawMode at g_PrimCursor with dfe 0
 *           and dtd 1, and links the 0xC-byte draw-mode packet at the record
 *           words 0x14 and 0x18 with kind 2. Then it publishes the scratchpad
 *           cells the loop reads: cell 0x1F800004 takes the record byte 0x06
 *           plus the low two bits of rand as the triangle radius, cells
 *           0x1F800000 and 0x1F800002 take the record halfwords 0x20 and 0x22
 *           as the published centre of the burst, cells 0x1F800008,
 *           0x1F80000A and 0x1F80000C take the palette bytes at 0x801F0158,
 *           0x801F0159 and 0x801F015A indexed by the record byte 0x04 times
 *           twelve plus the record byte 0x03 times three, each multiplied by
 *           record byte 0x05, and cell 0x1F80000E takes record byte 0x05
 *           itself. The body then runs eight iterations of one triangle each:
 *           every iteration takes a fresh g_PrimCursor primitive, seeds it
 *           through func_8017A97C and marks it semi-transparent, puts the
 *           published centre into the primitive halfwords 0x08 and 0x0A, adds
 *           the func_801782FC and func_801783C8 samples of the counter shifted
 *           left 9, scaled by the radius cell and shifted right 12, into the
 *           primitive halfwords 0x10 and 0x12, increments the counter, adds the
 *           same two samples of the incremented counter into the primitive
 *           halfwords 0x18 and 0x1A, writes the palette triplet into bytes
 *           0x04, 0x05 and 0x06, writes the 0x1F80000E cell into bytes 0x0C,
 *           0x0D and 0x0E and into bytes 0x14, 0x15 and 0x16, and links the
 *           0x1C-byte POLY_G3 packet at the record words 0x14 and 0x18 with
 *           kind 2, until the counter reaches eight and the eighth-turn phase
 *           walk has covered the whole turn. It keeps a 0x30-byte frame for ra
 *           and the four saved registers 0x18 to 0x24, and writes no state
 *           outside the scratchpad cells and the primitives it emits.
 * @status partial
 * @match 95.52
 * @residual 192 of 201 instructions match at 804 bytes against 804. The frame,
 *           scratchpad base register, memory accesses, sampler arguments,
 *           palette arithmetic and packet links agree. Nine instructions
 *           exchange the counter and phase registers: the original keeps
 *           the counter in s2 and phase in s1, while this build keeps the
 *           counter in s1 and phase in s2.
 */
void drawEightRotatedTrianglesFromPublishedCenterTintedByPaletteRowAtWorkBytes3And4BmagicMagic16203_801EFCD8(
    void) {
  u16 *cell;
  s16 *radius;
  s32 phase;
  POLY_G3 *primitive;
  s32 tpage;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(*(s32 *)(D_801F17B0 + 0x14), *(s32 *)(D_801F17B0 + 0x18), 2,
                0xC);

  i = 0;
  radius = (s16 *)&D_1F800004;
  *radius = D_801F17B0[6] + (rand() & 3);
  *(u16 *)0x1F800000 = *(u16 *)(D_801F17B0 + 0x20);
  *(u16 *)0x1F800002 = *(u16 *)(D_801F17B0 + 0x22);
  *(u16 *)0x1F800008 =
      D_801F0158[D_801F17B0[4] * 12 + D_801F17B0[3] * 3] * D_801F17B0[5];
  *(u16 *)0x1F80000A =
      D_801F0159[D_801F17B0[4] * 12 + D_801F17B0[3] * 3] * D_801F17B0[5];
  *(u16 *)0x1F80000C =
      D_801F015A[D_801F17B0[4] * 12 + D_801F17B0[3] * 3] * D_801F17B0[5];
  *(u16 *)0x1F80000E = D_801F17B0[5];

  cell = (u16 *)radius - 2;
  do {
    primitive = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);

    primitive->x0 = cell[0];
    primitive->y0 = cell[1];
    phase = i << 9;
    primitive->x1 = cell[0] + ((func_801782FC(phase) * ((s16 *)cell)[2]) >> 12);
    primitive->y1 = cell[1] + ((func_801783C8(phase) * ((s16 *)cell)[2]) >> 12);
    phase = (i + 1) << 9;
    primitive->x2 = cell[0] + ((func_801782FC(phase) * ((s16 *)cell)[2]) >> 12);
    primitive->y2 = cell[1] + ((func_801783C8(phase) * ((s16 *)cell)[2]) >> 12);

    primitive->r0 = cell[4];
    primitive->g0 = cell[5];
    primitive->b0 = cell[6];
    primitive->r1 = cell[7];
    primitive->g1 = cell[7];
    primitive->b1 = cell[7];
    primitive->r2 = cell[7];
    primitive->g2 = cell[7];
    primitive->b2 = cell[7];

    func_80155A08(*(s32 *)(D_801F17B0 + 0x14), *(s32 *)(D_801F17B0 + 0x18), 2,
                  0x1C);
  } while (++i < 8);
}
