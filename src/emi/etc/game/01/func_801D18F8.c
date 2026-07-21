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
  volatile u8*                   phase_addr;
  s32                            i;
  s32                            x;
  s32                            marker;
  s32                            flags;
  s32                            a;
  s32                            sc;
  u8                             phase;
  u8*                            primitive;

  /* MATCHING_AID: anchor the banner state on the fade-phase byte. cc1
   * materializes &GAME_FRONT_FADE_PHASE once and derives the scroll base
   * (-15) and alpha cell (-13) from it via addiu, reproducing the original
   * lui/addiu v1 ; addiu s0,v1,-15 ; addiu s4,v1,-13 prologue. */
  phase_addr = &GAME_FRONT_FADE_PHASE;
  if (*phase_addr == 0) {
    return;
  }
  state = (volatile GameFrontBannerState*)(phase_addr - 15);
  alpha = (volatile u16*)(phase_addr - 13);

  sc = GAME_FRONT_BANNER_SCROLL + 2;
  x = 320 - sc;
  GAME_FRONT_BANNER_SCROLL = sc;

  for (i = 0, marker = 320; i < 4; i++, marker += 128, x += 255) {
    if (i < (s16)state->scroll / 640) {
      continue;
    }

    /* MATCHING_AID: read the dispatch phase through a non-volatile view so
     * cc1 does not emit an andi 0xff zero-extension before the == 1 / == 3
     * compares (the original lbu feeds bne directly). */
    phase = *(u8*)&state->phase;
    if (phase == 1) {
      a = state->alpha + 1;
      state->alpha = a;
      if ((s16)a >= 128) {
        state->alpha = 128;
        state->phase = 2;
      }
    } else if (phase == 3) {
      a = state->alpha - 1;
      state->alpha = a;
      if ((s16)a <= 0) {
        state->alpha = 0;
        state->phase = 0;
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
