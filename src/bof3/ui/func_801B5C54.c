#include "bof3/ui/game00_internal.h"

/* @behavior Scans the stride-8 cell-rectangle records reached from the world
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
 * @source 0x801B5C54
 * @status partial
 * @match 43.41
 * @residual non-exact live audit: 56/129 instructions; 516 original bytes versus 500 current. */

u8 *func_801B5C54(void) {
  u8  *world;
  u8  *table;
  u8  *rec;
  u8   count;
  s8   dx;
  s8   dy;
  s32  off;
  s32  i;
  s32  j;
  s32  k;
  s32  limit;
  u16  x;
  u16  y;

  world = (u8 *)D_8017F974[D_80143F00];
  count = *(u8 *)(world + 0x31);
  table = *(u8 **)(world + 0x2C);
  for (i = count; i >= 0; i--) {
    rec = table + i * 8;
    for (j = rec[3] - 1; j >= 0; j--) {
      limit = 1;
      if (g_game_work->speed_70 == 0) {
        off = g_game_work->route_index_08 * 2;
        dx = (s8)D_80181B00[off];
        dy = (s8)D_80181B01[off];
        x = *(u16 *)((u8 *)g_game_work + 0x36) + dx;
        y = *(u16 *)((u8 *)g_game_work + 0x3A) + dy;
        if (*(u16 *)((u8 *)g_game_work + 0x34) == 0) {
          limit = *(u16 *)((u8 *)g_game_work + 0x38) != 0;
        }
      } else {
        dx = ((s8 *)D_80181B00)[g_game_work->route_index_08 * 2];
        dy = ((s8 *)D_80181B01)[g_game_work->route_index_08 * 2];
        x = *(s16 *)((u8 *)g_game_work + 0x36) + (dx == 1 ? 2 : dx);
        y = *(s16 *)((u8 *)g_game_work + 0x3A) + (dy == 1 ? 2 : dy);
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
    }
  }
  return 0;
}
