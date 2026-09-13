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
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void updateBanner(void) {
  volatile GameFrontBannerState* state;
  volatile u16*                  alpha;
  volatile u8*                   phase_addr;

  s32 i;
  s32 marker;

  s32 v;
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
   * This does not select a saved register for i. */
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
