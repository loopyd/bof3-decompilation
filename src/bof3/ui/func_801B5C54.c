#include "bof3/ui/game00_internal.h"

/* @source 0x801B5C54
 * @behavior Scans the stride-8 cell-rectangle records reached from the world
 *           entry D_8017F974[D_80143F00] (+0x2C record pointer, +0x31 signed
 *           record count) from record `count` down to record 0. Per record and
 *           per length index (byte +3, counted down) the candidate cell is the
 *           work-area coordinate word built from the route step table
 *           D_80181B00 / D_80181B01, loaded unsigned when the work speed byte
 *           0x70 is clear and signed when it is set; a non-zero fraction in the
 *           coordinate halfwords 0x34 / 0x38 adds one extra candidate step.
 *           A record whose leading byte (+0) and shifted second byte (+1, or
 *           the leading byte when style bit 0x80 of +2 is set) equal the
 *           candidate is returned; a completed scan returns 0.
 * @status partial
 * @match 88.37
 * @residual Native comparison: 114/129 instructions, 516 original bytes versus
 * 500 current. Shared branches and body size still differ. Halfword candidate
 * arithmetic preserves every scan result; comparisons observe only the low
 * byte. */
u8 *func_801B5C54(void) {
  u8  *world;
  u8  *rec;
  u32  cursor_address;
  u8   count;
  s8   dx;
  s8   dy;
  s32  off;
  s32  i;
  s32  j;
  s32  k;
  s32  limit;
  s32  sx;
  s32  sy;
  u16  x;
  u16  y;
  s8   signed_dx;
  s8   signed_dy;

  world = (u8 *)D_8017F974[D_80143F00];
  count = *(u8 *)(world + 0x31);
  rec = *(u8 **)(world + 0x2C);
  i = count;
  if (i >= 0) {
    cursor_address = (u32)i * 8 + (u32)rec;
    do {
      j = ((u8 *)cursor_address)[3] - 1;
      rec = (u8 *)cursor_address;
      if (j >= 0) {
        do {
          if (g_game_work->speed_70 == 0) {
            limit = 1;
            off = g_game_work->route_index_08 * 2;
            dx = (s8)D_80181B00[off];
            dy = (s8)D_80181B01[off];
            x = *(u16 *)((u8 *)g_game_work + 0x36) + dx;
            y = *(u16 *)((u8 *)g_game_work + 0x3A) + dy;
            if (*(u16 *)((u8 *)g_game_work + 0x34) == 0) {
              limit = *(u16 *)((u8 *)g_game_work + 0x38) != 0;
            }
          } else {
            signed_dx = ((s8 *)D_80181B00)[g_game_work->route_index_08 * 2];
            sx = *(s16 *)((u8 *)g_game_work + 0x36);
            if (signed_dx != 1) {
              sx += signed_dx;
              x = sx;
            } else {
              x = sx + 2;
            }
            signed_dy = ((s8 *)D_80181B01)[g_game_work->route_index_08 * 2];
            sy = *(s16 *)((u8 *)g_game_work + 0x3A);
            if (signed_dy != 1) {
              sy += signed_dy;
              y = sy;
            } else {
              y = sy + 2;
            }
            if (g_game_work->route_index_08 == 1 ||
                g_game_work->route_index_08 == 5) {
              limit = *(u16 *)((u8 *)g_game_work + 0x34) != 0;
            } else {
              limit = *(u16 *)((u8 *)g_game_work + 0x38) != 0;
            }
          }
          for (k = 0; k <= limit; k++) {
            if (rec[2] & 0x80) {
              if ((u8)x == rec[0] && (u8)y == rec[1] + j) {
                return rec;
              }
            } else {
              if ((u8)x == rec[0] + j && (u8)y == rec[1]) {
                return rec;
              }
            }
            if (g_game_work->route_index_08 == 1 ||
                g_game_work->route_index_08 == 5) {
              x++;
            } else {
              y++;
            }
          }
        } while (--j >= 0);
      }
      i--;
      cursor_address -= 8;
    } while (i >= 0);
  }
  return 0;
}
