#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Random source of the radius and phase jitter this body applies per pass. */
extern int rand(void);

/* Scratchpad work-record pointer cell at 0x1F800044. The body re-reads the cell
 * once before the loop for the centre pair and the opening tint, and once per
 * byte 2 test inside the loop. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. The body materialises the symbol once
 * as a %hi/%lo pair, stores the seeded radius through that register and derives
 * the phase pointer four bytes above it, so the cell also serves as the base of
 * the shade cell at 0x1F800002. The generated undefined_syms_auto.txt row for
 * 0x1F800000 covers it; the cells at 0x1F800002, 0x1F800004 and 0x1F800006 are
 * reached through the pointers, not bound by name. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Shared phase samplers of the main executable, bound by this target's
 * generated undefined_funcs_auto.txt. Both take a 12-bit phase and return a
 * fixed-point sample in the scale the `>> 12` products below divide by. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed helper called on the fresh cursor primitive before this
 * body writes its fields. */
void func_8017AA94(void *arg0);

/* Shared graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* @source 0x801F05E8
 * @behavior Burst drawing step of this payload, alternating with the jittered
 *           spoke burst at 0x801F02C8 across the gathered call sites of the
 *           dispatcher at 0x801EEE7C, which seed its 12-bit start phase from
 *           rand in steps of 0x333 and pass the radius jitter mask 3. It takes
 *           that start phase and that mask and returns nothing.
 *           It picks the draw-mode texture page, 0xA5 when GetGraphType returns
 *           1 or 2 and 0x35 otherwise, calling the query once per test with no
 *           cached result, arms SetDrawMode on the primitive cursor
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, and links that
 *           0xC-byte packet under ordering-table slot 3. It then publishes the
 *           burst state in scratchpad: a radius of 0x10 at 0x1F800000, the
 *           start phase at 0x1F800004 and a tint of work record byte 9 scaled
 *           by six at 0x1F800006.
 *           Each pass publishes the low five bits of the pass counter shifted
 *           left seven at 0x1F800002, samples the shared sampler at that same
 *           value, multiplies the sample by rand masked with arg1, adds that
 *           product shifted right twelve to the radius and advances the phase
 *           by the low six bits of rand within 0xFFF, then draws one
 *           translucent Gouraud spoke into ordering-table slot 3 at 0x14 bytes:
 *           the near end sampled at the open phase and the far end at the
 *           advanced one, both offset from the work record screen pair at
 *           unsigned halfwords 0x2E and 0x30, a core colour of the tint cell at
 *           0x1F800006 repeated with its third of three in blue, and a rim
 *           colour of the same cell repeated the same way. When work record byte
 *           2 is below 3 the tint cell is re-derived from the current radius
 *           minus 8 scaled by six before the core colour and again from the
 *           radius minus 9 before the rim colour. The loop repeats while the
 *           radius stays at or above 0xC, so this burst runs two passes longer
 *           than the 0x10 floor of its twin at 0x801F02C8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentJitteredGouraudSpokesAroundWorkScreenPairFromPhaseWhileRadiusAtLeastTwelveWithFinePhaseStepBmagicMagic17203_801F05E8(
    s32 startPhase, s16 jitterMask) {
  POLY_G3 *prim;
  u16 *cells;
  u16 *radius;
  s16 *phase;
  u16 centreX;
  u16 centreY;
  u16 shadeField;
  s32 sample;
  s32 graph;
  s32 i;

  if (GetGraphType() == 1) {
    graph = 0xA5;
  } else if (GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(3, 0xC);

  radius = (u16 *)&D_1F800000;
  *radius = 0x10;
  phase = (s16 *)(radius + 2);
  *(u16 *)0x1F800004 = startPhase;
  centreX = FIELD_REF(u16, g_battle_work, 0x2Eu);
  centreY = FIELD_REF(u16, g_battle_work, 0x30u);
  *(u16 *)0x1F800006 = g_battle_work[9] * 6;

  /* The pre-loop store runs through the radius cell's own materialisation and
   * the in-loop tint stores run through this copy of the same base. */
  cells = radius;

  i = 1;
  do {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017AA94(prim);
    SetSemiTrans(prim, 1);

    prim->x0 = centreX + ((func_801782FC(phase[0]) * phase[-2]) >> 12);
    prim->y0 = centreY + ((func_801783C8(phase[0]) * phase[-2]) >> 12);

    /* The shade cell and the sampler argument carry the same value, and the
     * original materialises it twice: the cell is written from the record field
     * and the sampler is called with the narrow copy scaled up, so the field
     * read is narrowed once and shifted twice. */
    shadeField = i & 0x1F;
    ((u16 *)phase)[-1] = (i & 0x1F) << 7;
    sample = func_801782FC(shadeField << 7);
    ((u16 *)phase)[-2] =
        ((u16 *)phase)[-2] + ((sample * (rand() & jitterMask)) >> 12);
    ((u16 *)phase)[0] = (((u16 *)phase)[0] + (rand() & 0x3F)) & 0xFFF;

    prim->x1 = centreX + ((func_801782FC(phase[0]) * phase[-2]) >> 12);
    prim->y1 = centreY + ((func_801783C8(phase[0]) * phase[-2]) >> 12);

    if (g_battle_work[2] < 3) {
      cells[3] = (phase[-2] - 0x8) * 6;
    }

    prim->r0 = ((u16 *)phase)[1];
    prim->g0 = ((u16 *)phase)[1];
    prim->b0 = (s16)((u16 *)phase)[1] / 3;

    if (g_battle_work[2] < 3) {
      ((u16 *)phase)[1] = (phase[-2] - 0x9) * 6;
    }

    prim->r1 = cells[3];
    prim->g1 = cells[3];
    prim->b1 = (s16)cells[3] / 3;

    GpuAppendPrim(3, 0x14);
    i++;
  } while (phase[-2] >= 0xC);
}
