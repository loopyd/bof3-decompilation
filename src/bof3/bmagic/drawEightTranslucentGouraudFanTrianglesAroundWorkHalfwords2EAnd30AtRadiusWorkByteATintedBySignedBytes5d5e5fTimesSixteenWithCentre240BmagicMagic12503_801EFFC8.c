#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044. The cell is re-read at
 * every point the original re-materialises the address, because the primitive
 * stores of the loop may alias it. Another lifted source of this target binds
 * the same cell under this name. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword cells. The body materializes 0x1F800000 and
 * 0x1F800014 as symbol base addresses (`lui` plus `addiu`) and keeps the first
 * live across the whole loop, so both bind by name. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800014; /* @source 0x1F800014 @kind data */

/* Shared primitive seed helper at 0x8017A97C, called on the fresh cursor
 * primitive of every step before this body writes its fields. No in-image
 * symbol names it, so it keeps its address name. */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and the cosine
 * of a 12-bit phase in the fixed-point scale that the `>> 12` products below
 * undo. Both are called with phase zero for the seeded rim point and with the
 * step phase afterwards. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* Shared packet linker at 0x80155A08: it resolves the (x, y) cell through the
 * shared cell-size pair, links the primitive at g_PrimCursor with the given
 * kind and advances the cursor by the given size, which is why the body
 * re-reads g_PrimCursor at the top of every iteration. */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */

/* Naming and shape evidence: no extracted payload holds this 580-byte span
 * (a scan of every out/binaries payload for the body's own 16-byte `0xF0` and
 * three-`sb` run found only this target), so the clone lever is ineligible and
 * the body was seeded from the original bytes. The nearest exact sibling is the
 * same drawing step of emi/bmagic/magic129/03 at 0x801F030C, which names these
 * roles first: the same seed pair of the phase-zero rim point in scratchpad
 * 0x1F800014 and 0x1F800016, the same centre from work halfwords 0x2E and 0x30,
 * the same `do` walk of eight 0x200 phases, the same POLY_G3 member order, and
 * the same func_80155A08 link with kind 2 and size 0x1C. Its statement order,
 * operand forms and declaration style were transcribed; every literal, cell,
 * work offset and tint form here comes from this span's own bytes. */
/* @source 0x801EFFC8
 * @behavior Battle-magic drawing step reached only from the draw branch of
 *           0x801EFB8C, which links a 0xC-byte draw-mode packet, calls
 *           0x800AF764, then calls this body and 0x801F020C before it closes
 *           the draw mode. It takes no arguments and returns nothing.
 *           It publishes the work record's unsigned byte 0x0A as the radius
 *           halfword at scratchpad 0x1F800000, then seeds the phase-zero rim
 *           point at 0x1F800014 and 0x1F800016 with the work record's unsigned
 *           halfwords 0x2E and 0x30 plus the sine and the cosine sample of
 *           phase zero scaled by that radius and shifted right 12, which puts
 *           the first rim point one radius above the centre. It publishes the
 *           rim tint as three times the signed work bytes 0x5D, 0x5E and 0x5F
 *           shifted left four into the scratchpad halfwords 0x1F800006,
 *           0x1F800008 and 0x1F80000A.
 *           It then emits eight triangles, one per 0x200 step of the 0x1000
 *           circle, so the eight wedges close the disc. A triangle is a
 *           semi-transparent gouraud POLY_G3 seeded by 0x8017A97C whose first
 *           vertex is the work record centre and whose colour bytes 0x04, 0x05
 *           and 0x06 are fixed at 0xF0; its second vertex is the rim point the
 *           previous step left in 0x1F800014 and 0x1F800016 and its third is
 *           the point of the step phase, which is the work halfword 0x2E or
 *           0x30 plus the sampler sample scaled by the radius and shifted right
 *           12 and which is written back to the rim cells before it is stored.
 *           Both rim vertices take the published tint triplet in colour bytes
 *           0x0C to 0x0E and 0x14 to 0x16, read back from the scratchpad cells.
 *           Every step links its 0x1C-byte packet at the work record words 0x34
 *           and 0x38 with kind 2.
 *           The original 0x28-byte frame keeps $ra at 0x24 and $s0 to $s4 at
 *           0x10 to 0x20, and the body writes no state outside the scratchpad
 *           cells, the cursor and the primitives it emits.
 *
 *           Residual: the owning compiler's loop pass hoists the repeated
 *           centre-colour constant into a sixth callee-saved register. Its
 *           loop dump records `Insn 230: regno 150 (life 3), move-insn savings
 *           1  moved to 310`, so this body keeps 0xF0 in $s5 across the loop
 *           (a 0x30-byte frame, `sw`/`lw $s5`) and stores it three times, where
 *           the original re-materializes it per iteration as `li $v0,0xF0`. The
 *           same compiler leaves a three-use constant in the loop of
 *           drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairAtRadiusArgumentWithGreyCentre
 *           at 0x801F0A84 (`insn 221 ... savings 1 not desirable`), so the
 *           divergence is that pass's desirability threshold for this loop, not
 *           a statement spelling: chain, raw-byte, PsyQ setRGB0/1/2, `for`, and
 *           narrow-local forms of the colour block all measured the same body.
 * @status partial
 * @match 90.41
 * @residual 132/146 instructions; 584/580 bytes. The loop pass hoists the
 *           repeated 0xF0 centre colour into $s5 instead of re-materializing
 *           `li $v0,0xF0` inside the loop.
 */
void drawEightTranslucentGouraudFanTrianglesAroundWorkHalfwords2EAnd30AtRadiusWorkByteATintedBySignedBytes5d5e5fTimesSixteenWithCentre240BmagicMagic12503_801EFFC8(
    void) {
    u16 *cells;
    u16 *colours;
    u16 *point;
    u16 *edge;
    POLY_G3 *primitive;
    s32 phase;
    s32 i;
    s32 shade;

    cells = &D_1F800000;
    *cells = g_battle_work[0xA];

    point = &D_1F800014;
    *point =
        *(u16 *)(g_battle_work + 0x2E) +
        ((func_801782FC(0) * *(s16 *)cells) >> 12);
    *(u16 *)0x1F800016 =
        *(u16 *)(g_battle_work + 0x30) +
        ((func_801783C8(0) * *(s16 *)cells) >> 12);

    shade = *(s8 *)(g_battle_work + 0x5D);
    *(u16 *)0x1F800006 = shade << 4;
    shade = *(s8 *)(g_battle_work + 0x5E);
    *(u16 *)0x1F800008 = shade << 4;
    shade = *(s8 *)(g_battle_work + 0x5F);
    *(u16 *)0x1F80000A = shade << 4;

    edge = point;
    colours = cells;

    i = 1;
    do {
        primitive = (POLY_G3 *)g_PrimCursor;
        func_8017A97C(primitive);
        SetSemiTrans(primitive, 1);

        primitive->x0 = *(u16 *)(g_battle_work + 0x2E);
        primitive->y0 = *(u16 *)(g_battle_work + 0x30);
        primitive->x1 = edge[0];
        primitive->y1 = edge[1];

        phase = i << 9;
        edge[0] = *(u16 *)(g_battle_work + 0x2E) +
                  ((func_801782FC(phase) * *(s16 *)colours) >> 12);
        primitive->x2 = edge[0];
        edge[1] = *(u16 *)(g_battle_work + 0x30) +
                  ((func_801783C8(phase) * *(s16 *)colours) >> 12);
        primitive->y2 = edge[1];

        primitive->r0 = 0xF0;
        primitive->g0 = 0xF0;
        primitive->b0 = 0xF0;
        primitive->r1 = colours[3];
        primitive->g1 = colours[4];
        primitive->b1 = colours[5];
        primitive->r2 = colours[3];
        primitive->g2 = colours[4];
        primitive->b2 = colours[5];

        i++;
        func_80155A08(*(s32 *)(g_battle_work + 0x34),
                      *(s32 *)(g_battle_work + 0x38), 2, 0x1C);
    } while (i < 9);
}
