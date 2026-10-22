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
 * @status partial
 * @match 62.35
 * @residual Band-limit scheduling and route-scan allocation differ; current body is 632 bytes versus 648 original.
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
  s32 hi;
  s32 lo;
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

  m = *(u8*)((u8*)D_80146884 + 0x92);
  bandWork = D_1F800044;
  t = D_801E2AB8[bandWork[6]].unk_1B;
  spread = m * 0xC0000;
  shift = t * 15 << 16;
  do {
    hi = 0x3E0000 - spread;
    lo = 0x320000 - spread;
    hi += shift;
    lo -= shift;
  } while (0);
  if (0x19FFFF < hi) {
    if (0x3E0000 < hi) {
      hi = 0x3E0000;
    }
  } else {
    hi = 0x1A0000;
  }
  if (0x10FFFF < lo) {
    if (0x320000 < lo) {
      lo = 0x320000;
    }
  } else {
    lo = 0x110000;
  }

  yWork = D_1F800044;
  y = *(s32*)(yWork + 0x38);
  if (y >= hi) {
    if (*(s32*)(yWork + 0x10) > 0) {
      *(s32*)(yWork + 0x10) = -*(s32*)(yWork + 0x10);
    }
  } else if (y <= lo) {
    if (*(s32*)(yWork + 0x10) < 0) {
      *(s32*)(yWork + 0x10) = -*(s32*)(yWork + 0x10);
    }
  }

  scanWork = D_1F800044;
  offset = 0;
  for (i = 0; i < 8; i++) {
    sx = *(s32*)((u8*)D_80181AC0 + offset);
    sy = *(s32*)((u8*)D_80181AC4 + offset);
    magnitude = sx;
    if (magnitude < 0) {
      magnitude = -magnitude;
    }
    if (magnitude < 0x401) {
      sx = sx * 2;
    }
    magnitude = sy;
    if (magnitude < 0) {
      magnitude = -magnitude;
    }
    if (magnitude < 0x401) {
      sy = sy * 2;
    }
    if (*(s32*)(scanWork + 0x0C) == sx && *(s32*)(scanWork + 0x10) == sy) {
      break;
    }
    offset += 8;
  }

  scanWork[8] = i;
}
