#include "bof3/bof3.h"
#include "gpu/prim.h"

extern int rand(void);

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. The original re-materialises the cell with its own
 * lui/lw pair at every use, because each intervening call may republish the
 * pointer, so every read stays on this named cell as in the exact bmagic
 * siblings
 * dispatchWorkByte2HandlerThenInvokeUpdateAndDrawWhenWorkBytes0And2SetBmagicMagic09203_801EF840
 * and
 * drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairWithRandomRadiusTintedByWorkByte9TimesTenBmagicMagic09203_801F04BC,
 * which read the same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad profile cells. The signed halfword at 0x1F800000 holds the radius
 * and the one at 0x1F800002 the running tint; both are seeded once from work
 * byte 9 at their own constant addresses and read back in the loop through the
 * live bases the anchor below establishes. The anchor word at 0x1F800004 takes
 * the work record screen halfword 0x2E, the halfword at 0x1F800006 takes 0x30,
 * and the two halfwords at 0x1F800008 and 0x1F80000A hold the sampled radius
 * offsets. Every later access of those cells is reached from the anchor address
 * the original materialises once at 0x1F800004 and offsets by plus and minus
 * four, which is why the span keeps two live bases at 0x1F800000 and 0x1F800008
 * instead of re-forming each cell address. The radius read of the loop reaches
 * 0x1F800000 through the 0x1F800008 base minus eight, the signed form the
 * original issues, while the tint, the screen pair and the two sampled tone
 * cells are reached from the 0x1F800000 base. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */

/* Shared primitive linker: it takes the work record projection pair, the
 * primitive kind and the packet size, links the primitive at g_PrimCursor and
 * advances the cursor by that size. This body links one 0xC-byte draw-mode
 * packet and then four 0x34-byte POLY_GT4 packets per iteration, all with kind
 * 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared primitive seed at 0x8017A9E0, sitting in the SDK library slot between
 * SetPolyG4 at 0x8017A9CC and SetSprt8 at 0x8017A9F4. The object this body
 * builds is 0x34 bytes and every field it writes sits at the POLY_GT4 offsets
 * (colours at 0x4/0x10/0x1C/0x28, vertices at 0x8/0x14/0x20/0x2C, uv pairs at
 * 0xC/0x18/0x24/0x30, clut 0xE, tpage 0x1A), so the prototype takes the typed
 * pointer as the exact sibling of this overlay does. */
void func_8017A9E0(POLY_GT4 *arg0);

/* Shared phase sampler at 0x801782FC, the 0x801782FC member of the rotator pair
 * whose 0x801783C8 partner this body never calls. It returns the fixed-point
 * sample of a 12-bit phase, which this body multiplies by a scratchpad cell and
 * shifts right 12 at every one of its four calls. */
s32 func_801782FC(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EFB04
 * @behavior Textured quad band of this overlay's work-byte-2 handler group and
 *           the draw pass the step dispatcher at 0x801EF840 reaches from the
 *           untaken side of its byte test, after the shared main-executable
 *           update pass func_800AF764. That jal at 0x801EF8A8 is the only word
 *           of this payload, of the main executable or of any other extracted
 *           payload under out/binaries that holds this address, and no word of
 *           the fifteen-word handler run based at 0x801F07BC does, so the span
 *           takes no arguments and returns nothing through the jr $ra at
 *           0x801F0258.
 *           It first publishes the work record into scratchpad: the halfword at
 *           0x1F800004 takes work record halfword 0x2E, the one at 0x1F800006
 *           takes 0x30, the signed halfword at 0x1F800000 takes work byte 9 as
 *           the radius and the one at 0x1F800002 takes three times that byte as
 *           the running tint. It then runs twenty-eight iterations with the row
 *           index from 4 to 0x1F. Each iteration reads the shared graphic type
 *           twice and selects the draw-mode tpage argument 0xA5 when either
 *           answer is 1 or 2 and 0x35 otherwise, installs it at g_PrimCursor
 *           with dfe 0 and dtd 1, and links the 0xC-byte draw-mode packet at the
 *           work record projection words 0x34 and 0x38 with kind 2.
 *           It then samples the shared rotator four times at the phases
 *           (row + 1) * 64 and row * 64: the (row + 1) sample scaled by the
 *           radius goes to 0x1F800008 and the row sample scaled by the radius to
 *           0x1F80000A as the two halfword offsets every quad adds to or
 *           subtracts from its screen pair, the row sample scaled by the tint
 *           goes to 0x1F80000E, the tint is then replaced by three times work
 *           byte 9 plus the low four bits of rand and the (row + 1) sample
 *           scaled by that new tint goes to 0x1F80000C. The body writes all four
 *           cells and reads each of them back after the calls that follow, so no
 *           value crosses a call in a register.
 *           Each iteration then emits four semi-transparent POLY_GT4 quads at
 *           the scratchpad screen pair, all four on the same two screen rows:
 *           the row edge shared by vertices 0 and 1 is the screen y minus
 *           (row - 3) times the tint and the edge shared by vertices 2 and 3 is
 *           the screen y minus (row - 4) times the tint, so the quad is as tall
 *           as the tint and the band steps one tint per row. The first quad puts
 *           vertex 1 and vertex 3 on the screen x and pulls vertices 0 and 2 left
 *           by the two radius offsets; the second quad mirrors it, putting
 *           vertices 0 and 2 on the screen x and pushing vertices 1 and 3 right
 *           by the same offsets; the third quad pulls every vertex left, by the
 *           0x1F80000C sample for vertices 0 and 2 and by the two radius offsets
 *           for vertices 1 and 3; and the fourth quad pushes every vertex right
 *           by the same four amounts. Colours: the first quad tints vertices 0
 *           and 2 with 0xA0 0x20 0x20 and vertices 1 and 3 with 0x80 0x80 0x20,
 *           the second swaps those two pairs, the third gives vertices 0 and 2
 *           the grey 1 and vertices 1 and 3 the 0x80 0x20 0x20 pair, and the
 *           fourth swaps those. Every quad takes the clut GetClut(0, 0x1FA) and
 *           the texture page GetTPage(0, 1, 0x340, 0x100), and its own four uv
 *           pairs come from the four cells as unsigned bytes: the v pair of
 *           vertices 0 and 1 is (row - 3) * 6 + 0x20 plus the tint on the first
 *           two quads and plus twice the tint on the last two, the v pair of
 *           vertices 2 and 3 is the same expression built on (row - 4), and the u
 *           byte at each vertex is either 0x80, the cell byte minus 0x80, or
 *           0x80 minus the cell byte, per quad and per vertex as the bytes show.
 *           Finally it links the finished 0x34-byte quad at the same record words
 *           with kind 2.
 *           The 0x60-byte frame keeps ra at 0x5C, fp at 0x58, s0 to s7 at 0x38 to
 *           0x54 and the tint at 0x18, and the body writes no state outside the
 *           scratchpad cells, the cursor and the primitives it emits; the work
 *           record offsets and the cell roles stay address-anchored.
 * @status partial
 * @match 97.25
 * @residual Texel-coordinate scheduling in the first and third quads. The build
 *           matches 459 instructions against the original 471 and emits 472
 *           instructions, or 1888 bytes against 1884. The first quad retains
 *           the original second tint reload and repeated -128 materialisation,
 *           but advances the second row calculation and constant-u stores.
 *           The third quad advances its v stores and leaves a load-delay nop.
 */
void drawTranslucentTexturedQuadBandOfFourQuadsPerRowSteppingSixTexelsFromRowFourToThirtyOneScaledByWorkByte9AndWorkByteABmagicMagic09203_801EFB04(
    void) {
  POLY_GT4 *primitive;
  u16 *mid;
  s16 *sample;
  u16 *cells;
  s32 tint;
  s32 tpage;
  s32 vrow;
  s32 vrow2;
  s32 ub;
  s32 i;

  mid = &D_1F800004;
  cells = mid + 2;
  sample = (s16 *)(mid - 2);
  *mid = *(u16 *)(g_battle_work + 0x2E);
  D_1F800006 = *(u16 *)(g_battle_work + 0x30);
  tint = g_battle_work[0xA];
  D_1F800000 = g_battle_work[9];
  D_1F800002 = g_battle_work[9] * 3;

  for (i = 4; i < 0x20; i++) {
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      tpage = 0xA5;
    } else {
      tpage = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                  2, 0xC);

    cells[0] = (func_801782FC((i + 1) * 64) * *(s16 *)(cells - 4)) >> 12;
    cells[1] = (func_801782FC(i * 64) * *(s16 *)(cells - 4)) >> 12;
    sample[7] = (func_801782FC(i * 64) * sample[1]) >> 12;
    sample[1] = g_battle_work[9] * 3 + (rand() & 0xF);
    sample[6] = (func_801782FC((i + 1) * 64) * sample[1]) >> 12;

    primitive = (POLY_GT4 *)g_PrimCursor;
    func_8017A9E0(primitive);
    SetSemiTrans(primitive, 1);
    primitive->x0 = *(u16 *)(sample + 2) - cells[0];
    primitive->x1 = *(u16 *)(sample + 2);
    primitive->x2 = *(u16 *)(sample + 2) - cells[1];
    primitive->x3 = *(u16 *)(sample + 2);
    primitive->y0 = primitive->y1 = *(u16 *)(sample + 3) - (i - 3) * tint;
    primitive->y2 = primitive->y3 = *(u16 *)(sample + 3) - (i - 4) * tint;
    primitive->r0 = 0xA0;
    primitive->g0 = 0x20;
    primitive->b0 = 0x20;
    primitive->r1 = 0x80;
    primitive->g1 = 0x80;
    primitive->b1 = 0x20;
    primitive->r2 = 0xA0;
    primitive->g2 = 0x20;
    primitive->b2 = 0x20;
    primitive->r3 = 0x80;
    primitive->g3 = 0x80;
    primitive->b3 = 0x20;
    primitive->clut = GetClut(0, 0x1FA);
    primitive->tpage = GetTPage(0, 1, 0x340, 0x100);
    vrow = (i - 3) * 6 + 0x20;
    vrow2 = (i - 4) * 6 + 0x20;
    primitive->u1 = 0x80;
    ub = -0x80 - *(u8 *)cells;
    primitive->v0 = vrow + tint;
    primitive->v1 = vrow + tint;
    primitive->u0 = ub;
    primitive->u3 = 0x80;
    ub = -0x80 - *(u8 *)(cells + 1);
    primitive->v2 = vrow2 + tint;
    primitive->v3 = vrow2 + tint;
    primitive->u2 = ub;
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                  2, 0x34);

    primitive = (POLY_GT4 *)g_PrimCursor;
    func_8017A9E0(primitive);
    SetSemiTrans(primitive, 1);
    primitive->x0 = *(u16 *)(sample + 2);
    primitive->x1 = *(u16 *)(sample + 2) + cells[0];
    primitive->x2 = *(u16 *)(sample + 2);
    primitive->x3 = *(u16 *)(sample + 2) + cells[1];
    primitive->y0 = primitive->y1 = *(u16 *)(sample + 3) - (i - 3) * tint;
    primitive->y2 = primitive->y3 = *(u16 *)(sample + 3) - (i - 4) * tint;
    primitive->r0 = 0x80;
    primitive->g0 = 0x80;
    primitive->b0 = 0x20;
    primitive->r1 = 0xA0;
    primitive->g1 = 0x20;
    primitive->b1 = 0x20;
    primitive->r2 = 0x80;
    primitive->g2 = 0x80;
    primitive->b2 = 0x20;
    primitive->r3 = 0xA0;
    primitive->g3 = 0x20;
    primitive->b3 = 0x20;
    primitive->clut = GetClut(0, 0x1FA);
    primitive->tpage = GetTPage(0, 1, 0x340, 0x100);
    primitive->u0 = 0x80;
    primitive->v0 = vrow + tint;
    ub = *(u8 *)cells - 0x80;
    primitive->u1 = ub;
    primitive->v1 = vrow + tint;
    primitive->u2 = 0x80;
    primitive->v2 = vrow2 + tint;
    ub = *(u8 *)(cells + 1) - 0x80;
    primitive->u3 = ub;
    primitive->v3 = vrow2 + tint;
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                  2, 0x34);

    primitive = (POLY_GT4 *)g_PrimCursor;
    func_8017A9E0(primitive);
    SetSemiTrans(primitive, 1);
    primitive->x0 = *(u16 *)(sample + 2) - *(u16 *)(sample + 6);
    primitive->x1 = *(u16 *)(sample + 2) - cells[0];
    primitive->x2 = *(u16 *)(sample + 2) - *(u16 *)(sample + 7);
    primitive->x3 = *(u16 *)(sample + 2) - cells[1];
    primitive->y0 = primitive->y1 = *(u16 *)(sample + 3) - (i - 3) * tint;
    primitive->y2 = primitive->y3 = *(u16 *)(sample + 3) - (i - 4) * tint;
    primitive->r0 = 1;
    primitive->g0 = 1;
    primitive->b0 = 1;
    primitive->r1 = 0x80;
    primitive->g1 = 0x20;
    primitive->b1 = 0x20;
    primitive->r2 = 1;
    primitive->g2 = 1;
    primitive->b2 = 1;
    primitive->r3 = 0x80;
    primitive->g3 = 0x20;
    primitive->b3 = 0x20;
    primitive->clut = GetClut(0, 0x1FA);
    primitive->tpage = GetTPage(0, 1, 0x340, 0x100);
    vrow += tint * 2;
    vrow2 += tint * 2;
    primitive->v0 = vrow;
    ub = -0x80 - *(u8 *)(sample + 6);
    primitive->u0 = ub;
    primitive->v1 = vrow;
    ub = -0x80 - *(u8 *)cells;
    primitive->u1 = ub;
    primitive->v2 = vrow2;
    ub = -0x80 - *(u8 *)(sample + 7);
    primitive->u2 = ub;
    primitive->v3 = vrow2;
    ub = -0x80 - *(u8 *)(cells + 1);
    primitive->u3 = ub;
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                  2, 0x34);

    primitive = (POLY_GT4 *)g_PrimCursor;
    func_8017A9E0(primitive);
    SetSemiTrans(primitive, 1);
    primitive->x0 = *(u16 *)(sample + 2) + cells[0];
    primitive->x1 = *(u16 *)(sample + 2) + *(u16 *)(sample + 6);
    primitive->x2 = *(u16 *)(sample + 2) + cells[1];
    primitive->x3 = *(u16 *)(sample + 2) + *(u16 *)(sample + 7);
    primitive->y0 = primitive->y1 = *(u16 *)(sample + 3) - (i - 3) * tint;
    primitive->y2 = primitive->y3 = *(u16 *)(sample + 3) - (i - 4) * tint;
    primitive->r0 = 0x80;
    primitive->g0 = 0x20;
    primitive->b0 = 0x20;
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r2 = 0x80;
    primitive->g2 = 0x20;
    primitive->b2 = 0x20;
    primitive->r3 = 1;
    primitive->g3 = 1;
    primitive->b3 = 1;
    primitive->clut = GetClut(0, 0x1FA);
    primitive->tpage = GetTPage(0, 1, 0x340, 0x100);
    ub = -0x80 - *(u8 *)cells;
    primitive->u0 = ub;
    primitive->v0 = vrow;
    ub = -0x80 - *(u8 *)(sample + 6);
    primitive->u1 = ub;
    primitive->v1 = vrow;
    ub = -0x80 - *(u8 *)(cells + 1);
    primitive->u2 = ub;
    primitive->v2 = vrow2;
    ub = -0x80 - *(u8 *)(sample + 7);
    primitive->u3 = ub;
    primitive->v3 = vrow2;
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                  2, 0x34);
  }
}
