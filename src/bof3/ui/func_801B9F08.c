#include "bof3/ui/game00_internal.h"

/* @source 0x801B9F08
 * @behavior Maps a one to three byte command sequence to a single command
 *           code: counts the leading nonzero bytes of the local copy,
 *           remaps them through the scratchpad byte table at 0x1F800000,
 *           resolves the 0x70/0xFF/0xB0 command families and then the
 *           0xA0-0xA3 and 0x20 families against the work-area route bit,
 *           returning 0 when no family matches.
 * @status partial
 * @match 79.19
 * @residual First differing byte is +0x00A0, the arm-dispatch jump target (original j 0x801ba26c, current j 0x801ba270): the current build is one instruction long (221 insns / 884 bytes vs 220 / 880). The count, remap, 0x70, 0xFF, 0xB0 and both dispatch arms match; the remaining difference is gcc branch-inversion and return-block layout in the odd/even dispatch and tail, plus its delay-slot filling of the n == 1 probe.
 */
s32 func_801B9F08(u8 arg0, u8 arg1, u8 arg2) {
  u8 buf[3];
  u8 *p;
  u8 n;
  s32 i;

  buf[0] = arg0;
  buf[1] = arg1;
  buf[2] = arg2;
  n = 0;
  for (i = 0; i < 3; i++) {
    if (buf[i] == 0) {
      break;
    }
    n++;
  }
  for (i = 0; i < n; i++) {
    buf[i] = D_1F800000[buf[i]];
  }
  if (n == 2 && buf[0] == 0x70 && buf[1] == buf[0]) {
    return 0x70;
  }
  for (i = 0; i < n; i++) {
    if (buf[i] == 0x70) {
      buf[i] = 0xFF;
    }
  }
  for (i = 0; i < n; i++) {
    if (buf[i] == 0xFF) {
      return 0x10;
    }
  }
  i = 0;
  while (i < n) {
    if (buf[i] != 0xB0) {
      break;
    }
    i++;
  }
  if (i == n) {
    return 0xB0;
  }
  if (n & 1) {
    for (i = 0; i < n; i++) {
      if (!(g_game_work->route_index_08 & 1) && buf[i] == 0xA2) {
        return 0x10;
      }
      if (buf[i] == 0xA3 || buf[i] == 0xA0) {
        return 0x10;
      }
    }
    if (n == 1 && (buf[0] & 0xF0) == 0xA0) {
      return buf[0];
    }
  } else {
    if ((buf[0] & 0xF0) == 0xA0 || (buf[1] & 0xF0) == 0xA0) {
      if (buf[0] == 0xA2 && buf[1] == buf[0]) {
        if (g_game_work->route_index_08 & 1) {
          return 0xA2;
        }
        return 0x10;
      }
      if (buf[0] == 0xA3 && buf[1] == buf[0]) {
        return 0xA3;
      }
      if ((buf[0] & 0xF0) != 0xA0) {
        return 0x10;
      }
      if ((buf[1] & 0xF0) != (buf[0] & 0xF0)) {
        return 0x10;
      }
      return 0xA1;
    }
  }
  if (n < 2) {
    return 0;
  }
  for (i = 0; i < n - 1; i++) {
    if ((buf[i] & 0xF0) != 0x20) {
      continue;
    }
    p = &buf[i];
    for (i = 0; i < n; i++) {
      if ((buf[i] & 0xF0) == 0x20 && buf[i] != *p) {
        return 0x20;
      }
    }
    i = n;
  }
  return 0;
}
