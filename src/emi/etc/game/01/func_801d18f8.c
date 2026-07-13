#include "internal.h"

typedef struct GameFrontBannerState {
  u16 scroll;
  u16 alpha;
  u8  reserved_04[11];
  u8  phase;
} GameFrontBannerState;

/* @behavior advances the four-panel frontend banner fade and draws each
 * visible panel with its current alpha.
 * @source 0x801d18f8 func_801d18f8
 */
void func_801d18f8(void) {
  volatile GameFrontBannerState* state;
  vu16*                          alpha;
  s32                            i;
  s32                            x;
  s32                            marker;
  s32                            flags;
  u8*                            primitive;

  state = VPTR(GameFrontBannerState, 0x80143c22u);
  if (state->phase == 0) {
    return;
  }

  alpha = &state->alpha;
  state->scroll += 2;
  x = 320 - state->scroll;

  for (i = 0, marker = 320; i < 4; i++, marker += 128, x += 255) {
    if (i < (s16)state->scroll / 640) {
      continue;
    }

    if (state->phase == 1) {
      (*alpha)++;
      if ((s16)*alpha >= 128) {
        *alpha = 128;
        state->phase = 2;
      }
    } else if (state->phase == 2) {
      (*alpha)--;
      if ((s16)*alpha <= 0) {
        *alpha = 0;
        state->phase = 0;
      }
    } else {
      *alpha = 128;
    }

    if (state->phase == 0) {
      continue;
    }

    if (func_8017b2b4() == 1) {
      flags = ((marker & 0x3ff) >> 6) | 0x200;
    } else if (func_8017b2b4() == 2) {
      flags = ((marker & 0x3ff) >> 6) | 0x200;
    } else {
      flags = ((marker & 0x3ff) >> 6) | 0x80;
    }
    func_8017c2d8(DAT_8014598c, 0, 0, flags, 0);

    func_8014e5a0(2, 12);
    primitive = func_801d17d8((s16)x, 24, (u8)(i + 11), 2, 0);
    primitive[4] = *alpha;
    primitive[5] = *alpha;
    primitive[6] = *alpha;
  }
}
