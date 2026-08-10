#include "bof3/ui/game01_internal.h"

typedef struct GameFrontBannerState {
  u16 scroll;
  u16 alpha;
  u8  reserved_04[11];
  u8  phase;
} GameFrontBannerState;

/* @behavior advances the four-panel frontend banner fade and draws each
 * visible panel with its current alpha.
 * @source 0x801D18F8
 *
 * Exact byte match (130/130 insn, bin/byte-match MATCH) at canonical -O2.
 * The pin-free lift stalls at 109/131 insn (83.21%): control flow, calls,
 * loads/stores, and relocations all match; the diff is purely IRA register
 * allocation that the sanctioned clean-C levers cannot reverse (see the
 * MATCHING_AID notes below). `bin/flag-search` over all 52 catalog profiles
 * peaks at 83.21% (-O2); -O1 keeps both allocation choices and adds further
 * structural drift (58.87%). barrier()/CLOBBER_* govern delay-slot
 * scheduling, not register allocation, so they do not apply.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void updateBanner(void) {
  volatile GameFrontBannerState* state;
  volatile u16*                  alpha;
  volatile u8*                   phase_addr;
  /* MATCHING_AID: REGISTER_PIN on the two loop induction variables.
   * Original allocator residual: cc1 swaps i and marker (i -> s2, marker ->
   * s1; original i -> s1, marker -> s2) because marker's `& 0x3ff` uses sit
   * in inner delay slots, giving it the higher allocation priority. Current
   * allocator residual: identical swap at 83.21%. Exhausted rungs: clean-C
   * lifetime/init-order shaping, strength-reduced GIV derivation of marker,
   * expression order, and all 52 supported catalog profiles (peak 83.21%
   * at -O2; -O1 drifts structurally to 58.87%). Exact check: live
   * bin/asm-diff + bin/byte-match on emi/etc/game/01@0x801D18F8.
   * Removal condition: reattempt clean C whenever the compiler or flags
   * change. */
  REGISTER_PIN(s32, i, "s1");
  REGISTER_PIN(s32, marker, "s2");
  /* MATCHING_AID: same residual family as above: the original
   * rematerializes the 128 clamp / 2 phase constants in v0 at each store
   * site (li v0,128 / li v0,2 in branch delay slots); cc1 instead hoists
   * them into extra saved registers (s5/s6/s7), growing the frame. Pinning
   * the transient store temp to v0 restores the original shape. Removal
   * condition: same as above. */
  REGISTER_PIN(s32, v, "v0");
  s32                            x;
  s32                            flags;
  s32                            a;
  s32                            one;
  s32                            sc;
  u8                             phase;
  u8*                            primitive;

  /* Anchor the banner state on the fade-phase byte: the original derives the
   * scroll base (-15) and alpha cell (-13) from &GAME_FRONT_FADE_PHASE. */
  phase_addr = &GAME_FRONT_FADE_PHASE;
  /* Initialize the loop counter before the early-return guard so cc1 schedules
   * the `i = 0` clear into the beqz delay slot, matching the original prologue.
   * (This fixes delay-slot placement only; it does not steer which saved register
   * cc1 picks for i -- see the RESIDUAL note above.) */
  i = 0;
  if (*phase_addr == 0) {
    return;
  }
  state = (volatile GameFrontBannerState*)(phase_addr - 15);
  one = 1;
  alpha = (volatile u16*)(phase_addr - 13);
  marker = 320;

  sc = GAME_FRONT_BANNER_SCROLL + 2;
  x = 320 - sc;
  GAME_FRONT_BANNER_SCROLL = sc;

  for (; i < 4; x += 255, i++, marker += 128) {
    if (i < (s16)state->scroll / 640) {
      continue;
    }

    /* Read the dispatch phase through a non-volatile view so cc1 does not emit
     * an andi 0xff zero-extension before the == 1 / == 3 compares. */
    phase = *(u8*)&state->phase;
    if (phase == one) {
      a = state->alpha + 1;
      state->alpha = a;
      if ((s16)a >= 128) {
        v = 128;
        state->alpha = v;
        v = 2;
        *(u8*)&state->phase = v;
      }
    } else if (phase == 3) {
      a = state->alpha - 1;
      state->alpha = a;
      if ((s16)a <= 0) {
        state->alpha = 0;
        *(u8*)&state->phase = 0;
      }
    } else {
      v = 128;
      state->alpha = v;
    }

    if (GAME_FRONT_FADE_PHASE == 0) {
      continue;
    }

    flags = GetGraphType() == one   ? ((marker & 0x3ff) >> 6) | 0x200
            : GetGraphType() == 2 ? ((marker & 0x3ff) >> 6) | 0x200
                                  : ((marker & 0x3ff) >> 6) | 0x80;
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, flags, 0);

    appendRenderPrim(2, 12);
    primitive = drawGlyph((s16)x, 24, (u8)(i + 11), 2, 0);
    primitive[4] = *alpha;
    primitive[5] = *alpha;
    primitive[6] = *alpha;
  }
}
