#include "internal.h"

/* @behavior draws both New Game/Load prompt panels, their labels and selection
 * marker; when the popup is open, pulses the active marker primitive.
 * @source 0x801d12cc FUN_801d12cc
 */
void func_801d12cc(u8 selected, u8 alpha) {
  u8* primitive;
  s32 marker_x;
  s32 pulse;
  s32 pulse_counter;

  marker_x = 47;
  if (func_8017b2b4() == 1) {
    marker_x = 143;
  } else if (func_8017b2b4() == 2) {
    marker_x = 143;
  }
  func_8017c2d8(D_8014598C, 0, 0, marker_x, 0);
  func_8014e5a0(1, 12);
  primitive = func_801d17d8(262, 130, 1, 1, selected);
  func_801d18e8(primitive, alpha);

  marker_x = 189;
  if (func_8017b2b4() == 1) {
    marker_x = 685;
  } else if (func_8017b2b4() == 2) {
    marker_x = 685;
  }
  func_8017c2d8(D_8014598C, 0, 0, marker_x, 0);
  func_8014e5a0(2, 12);
  primitive = func_801d17d8(12, 200, 8, 2, selected);
  func_801d18e8(primitive, alpha);

  marker_x = 189;
  if (func_8017b2b4() == 1) {
    marker_x = 685;
  } else if (func_8017b2b4() == 2) {
    marker_x = 685;
  }
  func_8017c2d8(D_8014598C, 0, 0, marker_x, 0);
  func_8014e5a0(2, 12);
  primitive = func_801d17d8(12, 212, 19, 2, selected);
  func_801d18e8(primitive, alpha);
  primitive = func_801d17d8(172, 212, 9, 2, selected);
  func_801d18e8(primitive, alpha);

  if ((GAME_FRONT_POPUP_WORD & GAME_FRONT_POPUP_PENDING_MASK) ==
      GAME_FRONT_POPUP_PENDING_OPEN) {
    primitive = func_801d17d8(48, 184, 7, 2, 0);
    pulse_counter = D_80143C2A + 1u;
    D_80143C2A = pulse_counter;
    pulse = (pulse_counter & 0x20u) != 0u
                ? -128 - ((pulse_counter & 0x1fu) << 2)
                : ((pulse_counter & 0x1fu) << 2);
    primitive[4] = pulse;
    primitive[5] = pulse;
    primitive[6] = pulse;
  } else {
    D_80143C2A = 0u;
  }
}
