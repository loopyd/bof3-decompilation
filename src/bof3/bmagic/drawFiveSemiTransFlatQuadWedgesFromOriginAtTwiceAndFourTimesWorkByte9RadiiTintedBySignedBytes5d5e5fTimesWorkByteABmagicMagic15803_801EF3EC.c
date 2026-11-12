#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044. The body re-reads the cell
 * after the shared packet linker so that the primitive stores of one step may
 * alias it, and every access is a `lui` plus an offset load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cells at 0x1F800000, doubled work byte 9, and at
 * 0x1F800002, the same byte quadrupled: the apex vertex scales by the second and
 * the two base vertices by the first. The original materializes the 0x1F800000
 * address with its own `lui`/`addiu` pair for the opening store and copies that
 * register into the cell base the wedge loop indexes, while every other read of
 * these two cells re-materializes the address, so the store is written through
 * the fixed address and the reads through the declared cells. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */

/* Scratchpad quad base at 0x1F800014: the four 8-byte vertices the projection
 * call reads, the first of which the original also names with a `lui`/`addiu`
 * pair and derives the second vertex from. */
extern s16 D_1F800014; /* @source 0x1F800014 @kind data */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and the cosine
 * of a 12-bit phase in the fixed-point scale the `>> 12` products undo. Both are
 * called once per vertex coordinate with no cached answer. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* Shared flat-quad initialiser at 0x8017A9A4, called on the fresh cursor
 * primitive before this body writes the vertex, colour and texture fields. No
 * in-image symbol names it, so the tag carries the address. */
extern void func_8017A9A4(void *primitive); /* @source 0x8017A9A4 */

/* Shared packet linker at 0x80155A08: it resolves the (x, y) screen cell
 * through the shared cell-size pair, links the primitive the cursor holds into
 * the given output list and advances the cursor by the given size. This body
 * links its 0xC-byte draw-mode packet once and the 0x18-byte flat quad of every
 * wedge, both into output list 2 from the work object's own screen pair. */
extern void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3); /* @source 0x80155A08 */

/* Shared graphic-type query feeding the draw-mode texture-page argument. */
s32 GetGraphType(void);

/* @source 0x801EF3EC
 * @behavior Battle-magic draw step reached only from 0x801EEF34, the overlay's
 *           work-byte-2 dispatcher, which calls it after 0x801EF2F0 and before
 *           PopMatrix on the path where the work object's bytes 0 and 2 are both
 *           non-zero. It takes no arguments and returns nothing.
 *           It picks the draw-mode texture page, 0xA5 when GetGraphType answers
 *           1 or 2 and 0x35 otherwise, calling it once per test with no cached
 *           answer, arms SetDrawMode on the primitive cursor g_PrimCursor with
 *           dfe 0, dtd 1 and a null RECT, and links that 0xC-byte packet into
 *           output list 2 from the work object's screen words 0x34 and 0x38.
 *           It then publishes the two radii and the tint the wedges scale by:
 *           the scratchpad halfword at 0x1F800000 takes work byte 9 doubled and
 *           the halfword at 0x1F800002 takes it quadrupled, and the three
 *           halfwords at 0x1F800006, 0x1F800008 and 0x1F80000A take the signed
 *           work bytes 0x5D, 0x5E and 0x5F multiplied by the unsigned work byte
 *           0x0A, so the tint fades as that countdown byte grows. The work
 *           pointer is read once for those five cells, each byte read again for
 *           its own cell because the cell stores may alias the record.
 *           Five semi-transparent flat quads follow, the wedge counter stepping
 *           by two from zero to eight. The wedge phase at 0x1F800004 is the
 *           running 0x332 step, and it is cleared on the last wedge so the fan
 *           closes back on phase zero. Each wedge takes the primitive the cursor
 *           holds, seeds it as a flat quad, marks it semi-transparent, and
 *           builds its four vertices: the first is the apex at the phase halfway
 *           between the wedge's two edges scaled by the quadrupled radius, the
 *           second is the wedge's opening edge scaled by the doubled radius, the
 *           third is its closing edge scaled by the doubled radius, and the
 *           fourth is the origin. The two edge phases run 0x332 apart from a
 *           0x199 start, so five wedges cover one turn. It projects the four
 *           vertices through RotTransPers4 into the quad's vertex pairs at 0x08,
 *           0x0C, 0x10 and 0x14 with two frame words as the depth and flag
 *           results, gives the three colour bytes the three tint halfwords read
 *           back from the scratchpad, and links the 0x18-byte packet for the same
 *           work screen pair and output list.
 *           The 0x58-byte frame keeps $ra at 0x54, $s0 to $s7 and $fp at 0x30 to
 *           0x50, the six outgoing argument words at 0x10 to 0x24 and the depth
 *           and flag words at 0x28 and 0x2C. The body writes nothing outside the
 *           scratchpad cells it publishes and the primitives it emits; the roles
 *           of work bytes 9, 0x0A and 0x5D to 0x5F beyond this geometry are not
 *           established, so they stay offset-anchored.
 *           Shape evidence, not clone proof: emi/bmagic/magic166/03 at
 *           0x801EF380 holds the same 0x31C-byte span, and the only two words
 *           that differ are its `j` immediates, each shifted by the -0x6C span
 *           delta, so one body is proven; that twin's own row is still `asm`, so
 *           there is no source to clone and the clone lever is declined. The
 *           statement order, operand forms and declaration style were transcribed
 *           from two exact siblings of the same draw group: the flat-quad band at
 *           0x801EFB34 of bmagic/magic088/03, which binds the same cursor, sampler
 *           pair, POLY_F4 projection and 0x18-byte link, and the quad ring at
 *           0x801EFC98 of bmagic/magic110/03, which binds the same scratchpad
 *           cell and pointer forms. Every literal, cell, work offset and phase
 *           here comes from this span's own bytes.
 *           The first seed measured 174 of 199 instructions at 198 instructions
 *           and 792 bytes, and its first difference was the register the
 *           scratchpad cell address was materialized into, not an instruction
 *           shape. The representation rung closed it: the two radius cells are
 *           read as declared cells so each read re-materializes its address, the
 *           opening store is written through the fixed address so the compiler
 *           keeps it a separate memory event and copies its address register
 *           into the cell base, and the wedge counter, the two edge phases and
 *           the cell base are defined in that order ahead of the loop. The
 *           measured runs behind that are recorded in the lane ledger.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFiveSemiTransFlatQuadWedgesFromOriginAtTwiceAndFourTimesWorkByte9RadiiTintedBySignedBytes5d5e5fTimesWorkByteABmagicMagic15803_801EF3EC(
    void) {
  POLY_F4 *prim;
  SVECTOR *shape;
  SVECTOR *mid;
  s16 *cells;
  s16 *wedge;
  u8 *work;
  s32 phaseA;
  s32 phaseB;
  s32 outer;
  s32 step;
  s32 tpage;
  long depth;
  long flag;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(FIELD_REF(s32, g_battle_work, 0x34),
                FIELD_REF(s32, g_battle_work, 0x38), 2, 0xC);

  work = g_battle_work;
  /* The cell is scratchpad state that the emitted primitives and later spans
   * read, so the opening fixed-address store stays a distinct memory event
   * instead of being folded into the pointer the wedge loop indexes. */
  *(volatile s16 *)&D_1F800000 = work[9] * 2;
  D_1F800002 = work[9] * 4;
  SPAD_REF(u16, 0x06u) = FIELD_REF(s8, work, 0x5D) * work[0xA];
  SPAD_REF(u16, 0x08u) = FIELD_REF(s8, work, 0x5E) * work[0xA];
  SPAD_REF(u16, 0x0Au) = FIELD_REF(s8, work, 0x5F) * work[0xA];

  step = 0;
  shape = (SVECTOR *)&D_1F800014;
  mid = shape + 2;
  outer = 0x332;
  phaseB = 0;
  phaseA = 0x199;
  cells = &D_1F800000;
  wedge = cells + 2;

  for (; step < 10; step += 2) {
    prim = (POLY_F4 *)g_PrimCursor;
    func_8017A9A4(prim);
    SetSemiTrans(prim, 1);
    shape[0].vx = (func_801782FC(phaseA) * D_1F800002) >> 12;
    shape[0].vy = (func_801783C8(phaseA) * D_1F800002) >> 12;
    shape[0].vz = 0;
    shape[1].vx = (func_801782FC(phaseB) * D_1F800000) >> 12;
    shape[1].vy = (func_801783C8(phaseB) * D_1F800000) >> 12;
    shape[1].vz = 0;
    if (step == 8) {
      cells[2] = 0;
    } else {
      cells[2] = outer;
    }
    outer += 0x332;
    mid[0].vx = (func_801782FC(wedge[0]) * wedge[-2]) >> 12;
    mid[0].vy = (func_801783C8(wedge[0]) * wedge[-2]) >> 12;
    mid[0].vz = 0;
    mid[1].vx = 0;
    mid[1].vy = 0;
    mid[1].vz = 0;
    RotTransPers4(mid - 2, mid - 1, mid, mid + 1, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                  &depth, &flag);
    prim->r0 = *(u16 *)(cells + 3);
    prim->g0 = *(u16 *)(cells + 4);
    prim->b0 = *(u16 *)(cells + 5);
    phaseB += 0x332;
    phaseA += 0x332;
    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34),
                  FIELD_REF(s32, g_battle_work, 0x38), 2, 0x18);
  }
}
