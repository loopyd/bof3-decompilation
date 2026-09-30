#include "bof3/ui/game00_internal.h"

/* @source 0x801BA278
 * @behavior Maps a one to five byte command sequence to a single command code:
 *           counts the leading nonzero bytes of the local copy, remaps them
 *           through the scratchpad byte table at 0x1F800000, resolves the
 *           0x70/0xFF/0xB0 command families, then the two-byte and three-byte
 *           0xA0-0xA3 families against the work-area route bit, and finally the
 *           variable-length 0x20 family, returning 0 when no family matches.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801BA278(u8 arg0, u8 arg1, u8 arg2, u8 arg3, u8 arg4) {
  u8 buf[5];
  u8 n;
  s32 i;
  s32 j;
  u8* p;

  buf[0] = arg0;
  buf[1] = arg1;
  buf[2] = arg2;
  buf[3] = arg3;
  buf[4] = arg4;

  n = 0;
  for (i = 0; i < 5; i++) {
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
  if (n == 2) {
    if (buf[0] == 0xA0 || buf[1] == 0xA0) {
      return 0x10;
    }
    if ((buf[0] == 0xA2 || buf[1] == 0xA2) &&
        !(g_game_work->route_index_08 & 1)) {
      return 0x10;
    }
    if (buf[0] == 0xA3) {
      if (buf[1] == buf[0]) {
        return 0xA3;
      }
      return 0x10;
    }
    if ((buf[0] & 0xF0) == 0xA0 || (buf[1] & 0xF0) == 0xA0) {
      if (buf[0] == buf[1]) {
        return buf[0];
      }
      return 0x10;
    }
  } else if (n == 3) {
    if (buf[0] == 0xA3 || buf[1] == 0xA3 || buf[2] == 0xA3) {
      return 0x10;
    }
    if ((buf[0] == 0xA2 || buf[1] == 0xA2 || buf[2] == 0xA2) &&
        !(g_game_work->route_index_08 & 1)) {
      return 0x10;
    }
    if ((buf[0] & 0xF0) == 0xA0 || (buf[1] & 0xF0) == 0xA0 ||
        (buf[2] & 0xF0) == 0xA0) {
      if (buf[0] == 0xA0) {
        buf[0] = 0xA1;
      }
      if (buf[2] == 0xA0) {
        buf[2] = 0xA1;
      }
      if (buf[0] == buf[1] && buf[0] == buf[2]) {
        return buf[0];
      }
      return 0x10;
    }
  } else {
    for (i = 0; i < n; i++) {
      if ((buf[i] & 0xF0) == 0xA0) {
        return 0x10;
      }
    }
  }
  for (i = 0; i < n; i++) {
    if ((buf[i] & 0xF0) != 0x20) {
      continue;
    }
    p = &buf[i];
    for (j = i + 1; j < n; j++) {
      if ((buf[j] & 0xF0) == 0x20 && *p != buf[j]) {
        return 0x20;
      }
    }
    break;
  }
  return 0;
}
