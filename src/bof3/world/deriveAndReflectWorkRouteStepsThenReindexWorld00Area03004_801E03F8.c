#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E03F8
 * @behavior Load the route pair selected by work byte 8 into step words 0x0C and 0x10,
 * doubling routes 2 and 6. Reverse outward step signs at the x and derived y
 * limits. Scan eight shared pairs with the original small-magnitude doubling
 * and publish the first matching index or 8. This changes step signs, not the
 * position words.
 * @status exact
 * @match 100.00
 * @residual none
 */
void deriveAndReflectWorkRouteStepsThenReindexWorld00Area03004(void) {
  u8* routeWork;
  u8* xWork;
  u8* bandWork;
  u8* yWork;
  u8* scanWork;
  s32 spread;
  s32 shift;
  s32 m;
  s32 scaleByte;
  s32 ceiling;
  s32 x;
  s32 y;
  s32 sx;
  s32 sy;
  s32 magnitude;
  s32 i;
  s32 routeByteOffset;

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
  scaleByte = D_801E2AB8[bandWork[6]].unk_1B;
  spread <<= 18;
  sy = ceiling - spread;
  shift = scaleByte << 4;
  shift -= scaleByte;
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
  routeByteOffset = 0;
  for (; i < 8; i++) {
    sx = *(s32*)((u8*)D_80181AC0 + routeByteOffset);
    magnitude = (sx >= 0) ? sx : -sx;
    if (magnitude < 0x401) {
      sx = sx * 2;
    }
    sy = *(s32*)((u8*)D_80181AC4 + routeByteOffset);
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
    routeByteOffset += 8;
  }

  D_1F800044[8] = i;
}
