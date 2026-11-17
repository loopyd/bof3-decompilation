#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E03F8
 * @behavior Re-derives the movement step of the AREA030 work record published
 * at the scratchpad cursor 0x1F800044, confines it to the area's staging
 * rectangle and re-indexes the route table.
 * It first copies the route entry selected by record byte +0x08 out of the
 * global table pair 0x80181AC0 / 0x80181AC4 into the record step words at
 * +0x0C / +0x10, doubling both entries when that route index is 2 or 6.
 * The x step is then flipped so the record x at +0x34 heads back into the band
 * 0x80000..0x13FFFF (below it the step is forced non-negative, above it
 * non-positive), and the y step at +0x10 is flipped to keep the record y at
 * +0x38 inside the staging window [lo, hi] derived from the 0x80146884 byte
 * +0x92 (m, times 0xC0000) and the scale record D_801E2AB8[record byte +0x06]
 * byte +0x1B (t, times 15): hi = 0x3E0000 - m * 0xC0000 + (t * 15 << 16)
 * clamped to 0x1A0000..0x3E0000 and lo = 0x320000 - m * 0xC0000 - (t * 15 <<
 * 16) clamped to 0x110000..0x320000.
 * It finally scans the eight route entries for the first one whose step pair
 * (each word doubled when its magnitude is below 0x401) equals the record step
 * pair and stores that route index, or 8 when none matches, into record byte
 * +0x08.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E03F8(void) {
  u8* routeWork;
  u8* xWork;
  u8* bandWork;
  u8* yWork;
  u8* scanWork;
  s32 spread;
  s32 shift;
  s32 m;
  s32 t;
  s32 ceiling;
  s32 x;
  s32 y;
  s32 sx;
  s32 sy;
  s32 magnitude;
  s32 i;
  s32 offset;

  routeWork = D_1F800044;
  *(s32*)(routeWork + 0x0C) = D_80181AC0[routeWork[8] * 2];
  *(s32*)(routeWork + 0x10) = D_80181AC4[routeWork[8] * 2];
  if (routeWork[8] == 2 || routeWork[8] == 6) {
    *(s32*)(routeWork + 0x0C) = *(s32*)(routeWork + 0x0C) * 2;
    *(s32*)(routeWork + 0x10) = *(s32*)(routeWork + 0x10) * 2;
  }

  xWork = D_1F800044;
  x = *(s32*)(xWork + 0x34);
  if (x <= 0x80000) {
    if (*(s32*)(xWork + 0x0C) < 0) {
      *(s32*)(xWork + 0x0C) = -*(s32*)(xWork + 0x0C);
    }
  } else if (x > 0x13FFFF) {
    if (*(s32*)(xWork + 0x0C) > 0) {
      *(s32*)(xWork + 0x0C) = -*(s32*)(xWork + 0x0C);
    }
  }

  do {
    m = *(u8*)((u8*)D_80146884 + 0x92);
    i = 0x19FFFF;
    spread = m << 1;
    spread += m;
  } while (0);
  bandWork = D_1F800044;
  ceiling = 0x3E0000;
  t = D_801E2AB8[bandWork[6]].unk_1B;
  spread <<= 18;
  sy = ceiling - spread;
  shift = t << 4;
  shift -= t;
  shift <<= 16;
  do {
    sy += shift;
  } while (0);
  m = 0x320000 - spread;
  do {
    shift = m - shift;
  } while (0);
  i = i < sy;
  if (i != 0) {
    spread = (sy > ceiling) ? ceiling : sy;
  } else {
    spread = 0x1A0000;
  }
  sy = spread;
  spread = (shift < 0x110000) ? 0x110000 :
      ((shift > 0x320000) ? 0x320000 : shift);
  shift = spread;

  yWork = D_1F800044;
  y = *(s32*)(yWork + 0x38);
  if (y >= sy) {
    if (*(s32*)(yWork + 0x10) > 0) {
      *(s32*)(yWork + 0x10) = -*(s32*)(yWork + 0x10);
    }
  } else if (y <= shift) {
    if (*(s32*)(yWork + 0x10) < 0) {
      *(s32*)(yWork + 0x10) = -*(s32*)(yWork + 0x10);
    }
  }

  i = 0;
  scanWork = D_1F800044;
  offset = 0;
  for (; i < 8; i++) {
    sx = *(s32*)((u8*)D_80181AC0 + offset);
    magnitude = (sx >= 0) ? sx : -sx;
    if (magnitude < 0x401) {
      sx = sx * 2;
    }
    sy = *(s32*)((u8*)D_80181AC4 + offset);
    magnitude = (sy >= 0) ? sy : -sy;
    if (magnitude < 0x401) {
      sy = sy * 2;
    }
    if (*(s32*)(scanWork + 0x0C) == sx) {
      do {
        magnitude = *(s32*)(scanWork + 0x10);
      } while (0);
      if (magnitude == sy) {
        break;
      }
    }
    offset += 8;
  }

  D_1F800044[8] = i;
}
