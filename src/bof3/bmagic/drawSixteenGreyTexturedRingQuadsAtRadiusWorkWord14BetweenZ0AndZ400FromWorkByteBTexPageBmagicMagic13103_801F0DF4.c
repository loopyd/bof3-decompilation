#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The body loads the cell once for the radius and the three
 * tint lanes, then re-reads it for the semi-transparency flag and again for the
 * texture-page semi-transparency argument, each of which sits beyond the
 * primitive stores. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad ring cells at 0x1F800000. The signed halfword at 0x1F800000 is the
 * ring radius: the pre-loop seed truncates the work-object word at offset 0x14
 * into it through one 32-bit load, and every sample of both phase rotators is
 * multiplied by it and shifted right twelve. The three halfwords at 0x1F800008,
 * 0x1F80000A and 0x1F80000C hold the same grey tint, work-object byte 0x9 times
 * eight, one per colour lane. The original publishes the tint through absolute
 * `lui $at` plus `sh` pairs and reads it back as `lhu` offsets of the radius
 * base register, so the two access forms are separate spellings of the same
 * cells. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800008; /* @source 0x1F800008 @kind data */
extern s16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern s16 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Scratchpad x halfword of the first ring vertex at 0x1F80001C. The original
 * forms this address into a saved register with its own `lui 0x1F80` plus
 * `addiu 0x1C` pair and derives the four-vertex base by subtracting eight,
 * which is why the loop reaches the vectors through that base. The following
 * halfword at 0x1F80001E takes the same seed through the absolute `lui $at`
 * form. */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */
extern s16 D_1F80001E; /* @source 0x1F80001E @kind data */

/* The four scratchpad vectors at 0x1F800014 that RotTransPers4 projects. */
extern SVECTOR D_1F800014[]; /* @source 0x1F800014 @kind data */

/* Shared phase rotators of the main executable: the 0x801782FC sample feeds
 * every x halfword and the 0x801783C8 sample every y halfword, in the fixed
 * point scale the `>> 12` products below divide out. */
extern s32 func_801782FC(s32 arg0); /* @source 0x801782FC */
extern s32 func_801783C8(s32 arg0); /* @source 0x801783C8 */

/* Shared graphic-type query feeding the draw-mode texture-page argument. */
s32 GetGraphType(void);

/* @source 0x801F0DF4
 * @behavior Draws the sixteen textured quads of a translucent ring band around
 *           the radius the caller left in work-object word 0x14. It selects the
 *           draw-mode texture page from GetGraphType, whose two cases are the
 *           ones 0x25 and 0x15 name, and ORs the work object's low two bits of
 *           byte 0xB into bit 7 of the first case and bit 5 of the second;
 *           it arms SetDrawMode on the shared primitive cursor at 0x8014598C
 *           with dfe 0, dtd 1 and a null tw, and appends a 0xC-byte packet to
 *           output list 3 through the shared append routine at 0x8014E5A0. It
 *           then truncates work word 0x14 into the scratchpad radius halfword
 *           0x1F800000 and publishes three times work byte 0x9 shifted left
 *           three as the grey tint lanes at 0x1F800008, 0x1F80000A and
 *           0x1F80000C, and seeds the first ring vertex at 0x1F80001C and
 *           0x1F80001E from the phase-zero samples of that radius. The sixteen
 *           steps run the full 4096-unit turn in 0x100 strides: each step
 *           copies the previous sample into the vertices at 0x1F800014,
 *           0x1F80001C and 0x1F80002C, writes -0x400 as the depth of the first
 *           two vertices and zero as the depth of the other two, and stores the
 *           step's two samples scaled by the radius into 0x1F80001C and
 *           0x1F80001E with the same values copied to 0x1F80002C and 0x1F80002E.
 *           Each step then stamps a POLY_FT4 at the primitive cursor through
 *           SetPolyFT4, sets semi-transparency from work byte 0xB, writes
 *           GetTPage(1, work byte 0xB, 0x340, 0x100) and GetClut(0, 0x1FA),
 *           toggles the u coordinate between 0 and 0x40 with a 0x3F texel width
 *           and v between 0 and 0xBF by step parity, copies the tint lanes into
 *           the primitive's red, green and blue bytes, projects the four vectors
 *           into x0 to x3 through RotTransPers4, and appends the 0x28-byte
 *           primitive to output list 3 unless the step index lies in 7 to 14.
 *           It takes no arguments and returns nothing.
 *
 *           Shape evidence, not clone proof: no payload under out/binaries holds
 *           a byte-identical copy of this 0x300-byte span (a cmp of all emi
 *           payloads against the extracted bytes finds only this target's own
 *           offset 0x21F4), so the clone lever has no source to clone and the
 *           body is a clean-C seed. The statement order, the scratchpad access
 *           forms and the temporary reuse follow the same-target draw-group
 *           siblings drawUpToFourGreyRotatedTexturedQuadsScaledByPerSlotCounterBytesBmagicMagic06603_801EFCC8
 *           (single 0x1F800000 base pointer, 0x1F800014 vector array,
 *           RotTransPers4 into the primitive xy slots and the 0x28-byte append)
 *           and
 *           drawSemiTransFlatQuadHemisphereOfFourLatitudeBandsSixteenLongitudeStepsAtRadius32CentredZ448SkippingFourLeadingBmagicMagic10703_801EF544
 *           (the cell pointer formed into a register with the vector base
 *           derived from it by subtracting eight).
 * @status partial
 * @match 92.35
 * @residual 181/192 original instructions; 768->784 bytes. A loop-local radius pointer
 *           initialized from the scratchpad symbol gives the pre-loop radius,
 *           vector base and counter the payload's $s1/$s2/$s3 allocation. It
 *           also introduces a saved-pointer copy into $s4, moves the -0x400
 *           constant into $s1, and rematerializes the three tint addresses.
 *           Initializing that loop-local pointer from cells, or block-scoping
 *           the single radius pointer, measured the prior 155/192 allocation
 *           rotation at 768 bytes. Compiler and flags remain unchanged.
 */
void drawSixteenGreyTexturedRingQuadsAtRadiusWorkWord14BetweenZ0AndZ400FromWorkByteBTexPageBmagicMagic13103_801F0DF4(
    void) {
  POLY_FT4 *prim;
  SVECTOR *shape;
  u16 *next;
  s16 *cells;
  u8 *work;
  long p;
  long flag;
  s32 tpage;
  s32 phase;
  s32 u;
  s32 i;
  u16 vx;
  u16 vy;
  s32 ty;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = ((g_battle_work[0xB] & 3) << 7) | 0x25;
  } else {
    tpage = ((g_battle_work[0xB] & 3) << 5) | 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(3, 0xC);
  work = g_battle_work;
  cells = &D_1F800000;
  *cells = *(s32 *)(work + 0x14);
  D_1F800008 = work[9] << 3;
  D_1F80000A = work[9] << 3;
  D_1F80000C = work[9] << 3;
  next = &D_1F80001C;
  next[0] = (func_801782FC(0) * *cells) >> 12;
  D_1F80001E = (func_801783C8(0) * *cells) >> 12;
  shape = (SVECTOR *)(next - 4);
  for (i = 1; i < 17; i++) {
    s16 *radius = &D_1F800000;
    phase = i << 8;
    vx = shape[1].vx;
    vy = shape[1].vy;
    shape[0].vx = vx;
    shape[0].vy = vy;
    shape[2].vx = vx;
    shape[2].vy = vy;
    shape[0].vz = -0x400;
    shape[2].vz = 0;
    shape[1].vx = (func_801782FC(phase) * *radius) >> 12;
    ty = (func_801783C8(phase) * *radius) >> 12;
    shape[1].vy = ty;
    shape[3].vx = shape[1].vx;
    shape[3].vy = ty;
    shape[1].vz = -0x400;
    shape[3].vz = 0;
    prim = (POLY_FT4 *)g_PrimCursor;
    SetPolyFT4(prim);
    SetSemiTrans(prim, g_battle_work[0xB]);
    prim->tpage = GetTPage(1, g_battle_work[0xB], 0x340, 0x100);
    prim->clut = GetClut(0, 0x1FA);
    u = (i & 1) << 6;
    prim->u0 = u;
    prim->v0 = 0;
    prim->u1 = u + 0x3F;
    prim->v1 = 0;
    prim->u2 = u;
    prim->v2 = 0xBF;
    prim->u3 = u + 0x3F;
    prim->v3 = 0xBF;
    prim->r0 = ((u16 *)radius)[4];
    prim->g0 = ((u16 *)radius)[5];
    prim->b0 = ((u16 *)radius)[6];
    RotTransPers4(shape, shape + 1, shape + 2, shape + 3, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3, &p,
                  &flag);
    if (i < 7 || i > 14) {
      GpuAppendPrim(3, 0x28);
    }
  }
}
