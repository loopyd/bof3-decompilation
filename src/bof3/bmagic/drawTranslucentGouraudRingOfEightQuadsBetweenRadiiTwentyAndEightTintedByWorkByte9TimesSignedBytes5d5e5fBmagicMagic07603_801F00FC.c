#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, shared by every overlay
 * of this family. The loop re-reads the cell because the calls inside it may
 * replace the work object. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword cells at 0x1F800000, addressed here as one base. The
 * body materializes the base as a symbol address, uses it for the first radius
 * store and keeps a view of it, and of its second word, live across the whole
 * loop. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and the cosine
 * of a 12-bit phase in the fixed-point scale the `>> 12` products below
 * undo. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* Shared packet linker at 0x80155A08: it links the primitive at g_PrimCursor
 * through the work object's word pair 0x34/0x38 with the given kind and size
 * and advances the cursor, which is why the body re-reads g_PrimCursor at the
 * top of every step. */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */

/* Naming and shape evidence: an extracted payload holds this 728-byte span
 * once more, byte for byte, in emi/bmagic/magic169/03 at 0x801F0090 (one
 * `cmp` of the two 0x2D8-byte payload spans), but that row is still `asm`, so
 * it carries no source to clone and the clone lever is declined. No other
 * payload of the family holds the body's own seed block, so the seed was
 * transcribed from the shape siblings that share this drawing step: the
 * eight-step translucent quad ring of emi/bmagic/magic124/03 at 0x801F05B4
 * (exact, same POLY_G4 walk, same `do { } while` of eight 0x200 phases, same
 * func_80155A08 link with kind 2 and size 0x24) and the ring at
 * emi/bmagic/magic130/03 at 0x801EF730 (exact, same scratchpad radius cells
 * and `outer[0]`/`outer[-1]` reading form). Every literal, cell, work offset,
 * store order and tint form here comes from this span's own bytes. */
/* @source 0x801F00FC
 * @behavior Overlay drawing step of the work-byte-0/2 branch at 0x801EF8EC,
 *           which links a 0xC-byte draw-mode packet, calls 0x800AF764, then
 *           calls this body and 0x801EFEA8 before it closes the draw mode. It
 *           takes no arguments and returns nothing.
 *           It publishes the fixed radii 8 and 0x14 into scratchpad halfwords
 *           0x1F800000 and 0x1F800002 and the fade tint into 0x1F800008,
 *           0x1F80000A and 0x1F80000C, each the unsigned work byte 0x09 times
 *           the signed work bytes 0x5D, 0x5E and 0x5F, and caches the screen
 *           centre pair from the work record's unsigned halfwords 0x2E and
 *           0x30.
 *           It then emits eight semi-transparent Gouraud POLY_G4 quads, one
 *           per 0x200 step of the 0x1000 circle, so the eight quads close the
 *           ring between the two published radii. Each quad's vertices zero
 *           and two sample the step phase and its vertices one and three the
 *           next phase, the centre plus the sampler sample times the radius
 *           shifted right 12, with vertex zero and one on radius 0x14 and
 *           vertex two and three on radius 8. Vertices zero and one take RGB
 *           (1,1,1); vertices two and three take the low bytes of the three
 *           tint halfwords. Every step links its 0x24-byte packet at the work
 *           record words 0x34 and 0x38 with kind 2.
 *           The 0x38-byte frame keeps $ra at 0x34, $fp at 0x30 and $s0 to $s7
 *           at 0x10 to 0x2C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudRingOfEightQuadsBetweenRadiiTwentyAndEightTintedByWorkByte9TimesSignedBytes5d5e5fBmagicMagic07603_801F00FC(
    void) {
    s16 *cells;
    s16 *outer;
    u16 *colours;
    POLY_G4 *primitive;
    u16 centreX;
    u16 centreY;
    s32 phase;
    s32 nextPhase;
    s16 i;

    centreX = *(u16 *)(g_battle_work + 0x2E);
    centreY = *(u16 *)(g_battle_work + 0x30);
    cells = &D_1F800000;
    *cells = 8;
    *(u16 *)0x1F800002 = 0x14;
    *(u16 *)0x1F800008 = *(s8 *)(g_battle_work + 0x5D) * g_battle_work[9];
    *(u16 *)0x1F80000A = *(s8 *)(g_battle_work + 0x5E) * g_battle_work[9];
    *(u16 *)0x1F80000C = *(s8 *)(g_battle_work + 0x5F) * g_battle_work[9];
    i = 0;
    outer = cells + 1;
    colours = (u16 *)cells;

    do {
        primitive = (POLY_G4 *)g_PrimCursor;
        SetPolyG4(primitive);
        SetSemiTrans(primitive, 1);
        phase = i << 9;
        primitive->x0 = centreX + ((func_801782FC(phase) * outer[0]) >> 12);
        primitive->y0 = centreY + ((func_801783C8(phase) * outer[0]) >> 12);
        primitive->x2 = centreX + ((func_801782FC(phase) * outer[-1]) >> 12);
        primitive->y2 = centreY + ((func_801783C8(phase) * outer[-1]) >> 12);
        nextPhase = (i + 1) << 9;
        primitive->x1 = centreX + ((func_801782FC(nextPhase) * outer[0]) >> 12);
        primitive->y1 = centreY + ((func_801783C8(nextPhase) * outer[0]) >> 12);
        primitive->x3 = centreX + ((func_801782FC(nextPhase) * outer[-1]) >> 12);
        primitive->y3 = centreY + ((func_801783C8(nextPhase) * outer[-1]) >> 12);
        primitive->r0 = 1;
        primitive->g0 = 1;
        primitive->b0 = 1;
        primitive->r1 = 1;
        primitive->g1 = 1;
        primitive->b1 = 1;
        primitive->r2 = colours[4];
        primitive->g2 = colours[5];
        primitive->b2 = colours[6];
        primitive->r3 = colours[4];
        primitive->g3 = colours[5];
        primitive->b3 = colours[6];
        func_80155A08(*(s32 *)(g_battle_work + 0x34),
                      *(s32 *)(g_battle_work + 0x38), 2, 0x24);
        i++;
    } while (i < 8);
}
