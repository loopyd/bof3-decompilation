#include "internal.h"

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
 * RESIDUAL (pin-free, 109/131 insn = 83.21% at canonical -O2): control flow,
 * calls, loads/stores, and relocations all match; the remaining diff is purely
 * IRA register allocation that the sanctioned levers cannot reverse:
 *   1. cc1 assigns the two loop induction variables in the opposite order to the
 *      original (marker -> s1, i -> s2; original i -> s1, marker -> s2). marker
 *      carries the higher allocation priority (its `& 0x3ff` uses sit in the
 *      inner delay slots), so init-order, live-range shaping, and deriving marker
 *      as a strength-reduced GIV of i all leave the swap in place.
 *   2. cc1 hoists the loop-invariant clamp constant 128 into a 7th saved register
 *      (s5) and displaces the `1` compare constant to s6, instead of rematerial-
 *      izing 128 in the branch delay slots and saving only s0-s5 as the original
 *      does. Block-scoping the constant and the CSE-limiting catalog profiles do
 *      not stop the hoist.
 * `bin/flag-search` over all 52 catalog profiles peaks at 83.21% (-O2); -O1 keeps
 * both allocation choices and adds further structural drift (58.87%). barrier()/
 * CLOBBER_* govern delay-slot scheduling, not register allocation, so they do not
 * apply. Reaching 100% requires a register pin (banned) for i/marker/1/128.
 */
void func_801D18F8(void) {
  volatile GameFrontBannerState* state;
  volatile u16*                  alpha;
  volatile u8*                   phase_addr;
  s32                            i;
  s32                            x;
  s32                            marker;
  s32                            flags;
  s32                            a;
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
    if (phase == 1) {
      a = state->alpha + 1;
      state->alpha = a;
      if ((s16)a >= 128) {
        state->alpha = 128;
        *(u8*)&state->phase = 2;
      }
    } else if (phase == 3) {
      a = state->alpha - 1;
      state->alpha = a;
      if ((s16)a <= 0) {
        state->alpha = 0;
        *(u8*)&state->phase = 0;
      }
    } else {
      state->alpha = 128;
    }

    if (GAME_FRONT_FADE_PHASE == 0) {
      continue;
    }

    flags = GetGraphType() == 1 ? ((marker & 0x3ff) >> 6) | 0x200
            : GetGraphType() == 2 ? ((marker & 0x3ff) >> 6) | 0x200
                                  : ((marker & 0x3ff) >> 6) | 0x80;
    SetDrawMode((DR_MODE*)D_8014598C, 0, 0, flags, 0);

    func_8014E5A0(2, 12);
    primitive = func_801D17D8((s16)x, 24, (u8)(i + 11), 2, 0);
    primitive[4] = *alpha;
    primitive[5] = *alpha;
    primitive[6] = *alpha;
  }
}
