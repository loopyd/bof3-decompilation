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
 */
void func_801D18F8(void) {
  volatile GameFrontBannerState* state;
  volatile u16*                  alpha;
  s32                            i;
  s32                            x;
  s32                            marker;
  s32                            flags;
  u8*                            primitive;

  state = PSX_PTR(volatile GameFrontBannerState, 0x80143c22u);
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

    if (func_8017B2B4() == 1) {
      flags = ((marker & 0x3ff) >> 6) | 0x200;
    } else if (func_8017B2B4() == 2) {
      flags = ((marker & 0x3ff) >> 6) | 0x200;
    } else {
      flags = ((marker & 0x3ff) >> 6) | 0x80;
    }
    func_8017C2D8(D_8014598C, 0, 0, flags, 0);

    func_8014E5A0(2, 12);
    primitive = func_801D17D8((s16)x, 24, (u8)(i + 11), 2, 0);
    primitive[4] = *alpha;
    primitive[5] = *alpha;
    primitive[6] = *alpha;
  }
}
