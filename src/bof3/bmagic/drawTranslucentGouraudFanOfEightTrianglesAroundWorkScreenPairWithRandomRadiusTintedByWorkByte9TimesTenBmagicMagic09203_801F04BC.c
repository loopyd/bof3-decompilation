#include "bof3/bof3.h"
#include "gpu/prim.h"

extern int rand(void);

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. The original re-materializes the cell with its own
 * lui/lw pair at every use, because each intervening call may republish the
 * pointer, so every read stays on this named cell as in the exact bmagic
 * siblings
 * dispatchWorkByte2HandlerThenInvokeUpdateShiftHalfwords2EAnd30AndDrawWhenWorkBytes0And2SetBmagicMagic09203_801F0260
 * and
 * dispatchWorkByte2HandlerThenInvokeUpdateAndDrawWhenWorkBytes0And2SetBmagicMagic09203_801EF840,
 * which read the same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared engine primitive stamper at 0x8017A97C, called on the fresh cursor
 * primitive of every iteration before this body writes its fields. The payload
 * names that address only from this span, so the declaration keeps the raw name
 * and the generated undefined_funcs_auto.txt row for it. */
void func_8017A97C(void *arg0);

/* Shared primitive linker: it takes the work record projection pair, the
 * primitive kind and the packet size, links the primitive at g_PrimCursor and
 * advances the cursor by that size. This body links a 0xC-byte draw-mode packet
 * and then one 0x1C-byte POLY_G3 packet per triangle, both with kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the >> 12 products below. This body takes the 0x801782FC
 * sample for both vertex x sums and the 0x801783C8 sample for both vertex y
 * sums, the split its eight-triangle sibling
 * drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairTintedByWorkByteAWithJitteredRadiusBmagicMagic09703_801F083C
 * also takes. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* Scratchpad halfword at 0x1F800000, the signed radius every sampled offset of
 * the fan is scaled by and the base the tint cell at 0x1F80000E is reached
 * from. The original publishes it through the address of this cell, so the cell
 * is declared here and the generated undefined_syms_auto.txt row for
 * 0x1F800000 binds it. The tint cell itself is written at its own constant
 * address and read back at this base plus 0x0E. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Naming and shape evidence: the 668-byte span has no byte-identical twin - a
 * whole-span comparison against the 396 payloads under out/binaries finds it
 * only in this target's own binary - so the clone lever had no source to copy
 * and the body was seeded from the original bytes. The family twin at 0x801F01D0
 * in emi/bmagic/magic144/03 holds the same body with exactly one word changed,
 * the absolute j at 0x801F0558, and its own row is still asm, so it supplies
 * shape evidence and no source to clone. The nearest exact sibling is
 * drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairTintedByWorkByteAWithJitteredRadiusBmagicMagic09703_801F083C,
 * which carries the same eight-iteration POLY_G3 fan around the work record
 * screen pair, the same draw-mode disjunction over GetGraphType, the same 0xC
 * and 0x1C linker calls and the same jitter-scaled samples of the counter
 * shifted left 9; this body's own bytes supply the radius seed, the tint and
 * every literal. The radius cell is published through the address of its own
 * symbol and read back through the live signed halfword base the seed
 * establishes, so the radius store and the loop reads share one base register,
 * while the tint cell is written at its own constant address and read back at
 * that base plus 0x0E. */
/* @source 0x801F04BC
 * @behavior Translucent Gouraud triangle emitter of this overlay's work-byte-2
 *           handler group. Its only in-image caller is the draw branch of the
 *           dispatcher at 0x801F0260, which has just moved the work record
 *           halfword 0x2E by 0x10 in the direction of work byte 0xB and
 *           subtracted 0xC from halfword 0x30, and it reaches this body with no
 *           arguments, no result and no incoming register. It first seeds the
 *           scratchpad halfword at 0x1F800000 with the low four bits of rand
 *           plus 0x40, the radius every sampled offset of the fan is scaled by,
 *           and the scratchpad halfword at 0x1F80000E with ten times work byte
 *           9, the tint the centre vertex takes. It then runs eight iterations
 *           of one 0xC-byte draw-mode packet plus one POLY_G3 triangle: each
 *           iteration reads the shared graphic type twice and selects the
 *           draw-mode tpage argument 0xA5 when either answer is 1 or 2 and 0x35
 *           otherwise, installs it at g_PrimCursor with dfe 0 and dtd 1, and
 *           links the draw-mode packet at the work record projection words 0x34
 *           and 0x38 with kind 2. It then takes a fresh g_PrimCursor POLY_G3,
 *           stamps it through func_8017A97C, marks it semi-transparent, writes
 *           the work record screen halfwords 0x2E and 0x30 into the primitive
 *           halfwords 0x08 and 0x0A as the shared centre vertex, and adds the
 *           func_801782FC sample of the counter shifted left 9, scaled by the
 *           signed radius cell and shifted right 12, into halfword 0x10, with
 *           the func_801783C8 sample of the same phase into halfword 0x12;
 *           increments the counter; and adds the same two samples of the
 *           incremented counter into halfwords 0x18 and 0x1A, so neighbouring
 *           triangles meet at the shared sample and the eight of them span the
 *           whole turn. The centre vertex takes the low byte of the tint in
 *           colour byte 0x04 and the signed tint divided by five in bytes 0x05
 *           and 0x06, both rim vertices take one in bytes 0x0C, 0x0D, 0x0E,
 *           0x14, 0x15 and 0x16, and the body links the 0x1C-byte POLY_G3 packet
 *           at the same record words with kind 2 until the counter reaches
 *           eight. It keeps a 0x38-byte frame for ra at 0x30, the six saved
 *           registers 0x18 to 0x2C and the fifth draw-mode argument at 0x10, and
 *           writes no state outside the two scratchpad halfwords, the cursor and
 *           the primitives it emits; the two cells and the work record offsets
 *           stay address-anchored.
 * @status partial
 * @match 95.81
 * @residual one instruction: the original materialises the radius cell address
 *           through v1 and then copies it into the loop base (lui v1,%hi /
 *           addiu v1,v1,%lo / addu s3,v1,zero, which is why the store at
 *           0x801F0508 and every later read use s3 while v1 is reused for the
 *           work-record cell), whereas this source's direct pointer assignment
 *           materialises the same address straight into the base register
 *           (lui s3,%hi / addiu s3,s3,%lo). Every other instruction agrees, the
 *           six target words that differ are the branch and jump targets that
 *           shift with that one missing word, and the build is four bytes
 *           short. Every other spelling tried for that address - a raw constant,
 *           a symbol with and without an array declarator, byte and halfword
 *           pointer types with casts at the use sites, one-step and two-step
 *           pointer copies, integer intermediates, an explicit zero offset and
 *           a register declaration - produced the same two-word form.
 */
void drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairWithRandomRadiusTintedByWorkByte9TimesTenBmagicMagic09203_801F04BC(
    void) {
  POLY_G3 *primitive;
  s16 *radius;
  s32 phase;
  s32 tpage;
  s32 i;

  radius = &D_1F800000;
  *radius = (rand() & 0xF) + 0x40;
  *(u16 *)0x1F80000E = g_battle_work[9] * 10;
  i = 0;

  do {
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      tpage = 0xA5;
    } else {
      tpage = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                  FIELD_REF(s32, g_battle_work, 0x38u), 2, 0xC);

    primitive = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);
    primitive->x0 = FIELD_REF(u16, g_battle_work, 0x2Eu);
    primitive->y0 = FIELD_REF(u16, g_battle_work, 0x30u);
    phase = i << 9;
    primitive->x1 =
        FIELD_REF(u16, g_battle_work, 0x2Eu) +
        ((func_801782FC(phase) * *radius) >> 12);
    primitive->y1 =
        FIELD_REF(u16, g_battle_work, 0x30u) +
        ((func_801783C8(phase) * *radius) >> 12);
    i++;
    phase = i << 9;
    primitive->x2 =
        FIELD_REF(u16, g_battle_work, 0x2Eu) +
        ((func_801782FC(phase) * *radius) >> 12);
    primitive->y2 =
        FIELD_REF(u16, g_battle_work, 0x30u) +
        ((func_801783C8(phase) * *radius) >> 12);

    primitive->r0 = ((u16 *)radius)[7];
    primitive->g0 = (s16)((u16 *)radius)[7] / 5;
    primitive->b0 = (s16)((u16 *)radius)[7] / 5;
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r2 = 1;
    primitive->g2 = 1;
    primitive->b2 = 1;

    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                  FIELD_REF(s32, g_battle_work, 0x38u), 2, 0x1C);
  } while (i < 8);
}
