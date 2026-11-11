#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044. The body loads it once
 * into a local before the loop and reads every work offset through that local;
 * nothing inside the loop touches a work cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword cells. 0x1F800000 is formed as one `lui 0x1F80` plus
 * `addiu 0` and stays live in a register: the radius publish is a `sh` through
 * that base, and each of the four corner products re-reads the cell with
 * `lh 0(base)` because the primitive stores may alias it. The four tint cells
 * are published once through their own absolute addresses, each a `lui $at`
 * plus the store at the cell's own offset, so they bind by name at those
 * addresses rather than through the live base. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern u16 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Shared primitive seed helper at 0x8017A97C, called on the fresh cursor
 * primitive of every step before this body writes its fields. No map names it,
 * so it keeps its address name. */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the two samples of a
 * 12-bit phase in the fixed-point scale that the `>> 12` products below undo.
 * This body takes the 0x801782FC sample for every corner x and the 0x801783C8
 * sample for every corner y. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* Shared packet linker at 0x80155A08: it resolves the (x, y) cell through the
 * shared cell-size pair, links the primitive at g_PrimCursor with the given
 * kind and advances the cursor by the given size, which is why the body
 * re-reads g_PrimCursor at the top of every step. */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */

/* Naming and shape evidence: the span at 0x801EFE3C is byte-identical to the
 * still-unlifted asm row func_801EFEA8 of emi/bmagic/magic076/03 (one `cmp` of
 * the two payload spans, 596 bytes each, proved it), so that twin has no source
 * to clone and the adapter stays declined; this body was written from the
 * original bytes with the statement shape of the same drawing step of
 * emi/bmagic/magic125/03 at 0x801EFFC8, the family sibling that already names
 * these roles: the centre from work halfwords 0x2E and 0x30, the signed work
 * bytes 0x5D, 0x5E and 0x5F as the rim tint source, the scratchpad tint cells
 * 0x1F800006, 0x1F800008 and 0x1F80000A read back as colour bytes, the
 * func_8017A97C seed plus SetSemiTrans on each fresh POLY_G3, and the
 * func_80155A08 link with kind 2 and size 0x1C. Every literal, cell, work
 * offset, phase count and tint form here comes from this span's own bytes. */
/* @source 0x801EFE3C
 * @behavior Drawing step of this overlay with no in-image caller: no `jal`
 *           targets 0x801EFE3C and no word of the payload holds its address,
 *           so it is entered from outside the payload, takes no arguments and
 *           returns nothing.
 *           It loads the work object once and publishes the constant radius 8
 *           into scratchpad halfword 0x1F800000, then publishes four tint
 *           halfwords: 0x1F800006 is twelve times the work object's unsigned
 *           byte 9, and 0x1F800008, 0x1F80000A and 0x1F80000C are unsigned byte
 *           9 times the signed work bytes 0x5D, 0x5E and 0x5F.
 *           It then emits eight triangles, one per 0x200 step of the 0x1000
 *           circle, so the wedges close the disc. Each triangle is a fresh
 *           0x1C-byte POLY_G3 at g_PrimCursor, seeded through 0x8017A97C and
 *           marked semi-transparent. Its first corner is the work object's
 *           unsigned centre halfwords 0x2E and 0x30. Its second corner is that
 *           centre plus the two sampler answers at the step phase, and its
 *           third corner is the same centre plus the answers at the next phase,
 *           each product scaled by the published radius and shifted right 12 —
 *           x takes the 0x801782FC answer and y the 0x801783C8 answer.
 *           The centre colour bytes 4, 5 and 6 all take the 0x1F800006 cell
 *           (a grey shade), while the two rim vertices take the 0x1F800008,
 *           0x1F80000A and 0x1F80000C cells as their red, green and blue bytes,
 *           read back from the scratchpad through the live base pointer.
 *           Every step links its packet at the work object words 0x34 and 0x38
 *           with kind 2 through func_80155A08. The body writes no state outside
 *           the scratchpad cells, the cursor and the primitives it emits.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEightTranslucentGouraudFanTrianglesAroundWorkHalfwords2EAnd30AtRadiusEightTintedGreyByWorkByte9TimesTwelveWithRimFromSignedBytes5d5e5fTimesWorkByte9BmagicMagic16903_801EFE3C(
    void) {
    POLY_G3 *primitive;
    u8 *work;
    s16 *radius;
    u16 *colours;
    u16 centre_x;
    u16 centre_y;
    s32 phase;
    s32 next;
    s16 i;

    work = g_battle_work;
    radius = &D_1F800000;
    centre_x = *(u16 *)(work + 0x2E);
    centre_y = *(u16 *)(work + 0x30);
    *radius = 8;
    D_1F800006 = work[9] * 12;
    D_1F800008 = *(s8 *)(work + 0x5D) * work[9];
    D_1F80000A = *(s8 *)(work + 0x5E) * work[9];
    D_1F80000C = *(s8 *)(work + 0x5F) * work[9];
    i = 0;
    colours = (u16 *)radius;

    do {
        primitive = (POLY_G3 *)g_PrimCursor;
        func_8017A97C(primitive);
        SetSemiTrans(primitive, 1);
        phase = i << 9;
        primitive->x0 = centre_x;
        primitive->y0 = centre_y;
        primitive->x1 = centre_x + ((func_801782FC(phase) * *(s16 *)colours) >> 12);
        primitive->y1 = centre_y + ((func_801783C8(phase) * *(s16 *)colours) >> 12);
        next = (i + 1) << 9;
        primitive->x2 = centre_x + ((func_801782FC(next) * *(s16 *)colours) >> 12);
        primitive->y2 = centre_y + ((func_801783C8(next) * *(s16 *)colours) >> 12);
        primitive->r0 = colours[3];
        primitive->g0 = colours[3];
        primitive->b0 = colours[3];
        primitive->r1 = colours[4];
        primitive->g1 = colours[5];
        primitive->b1 = colours[6];
        primitive->r2 = colours[4];
        primitive->g2 = colours[5];
        primitive->b2 = colours[6];
        func_80155A08(*(s32 *)(g_battle_work + 0x34),
                      *(s32 *)(g_battle_work + 0x38), 2, 0x1C);
        i++;
    } while (i < 8);
}
