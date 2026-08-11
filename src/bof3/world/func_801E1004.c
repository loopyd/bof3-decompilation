#include "bof3/world/area03004_internal.h"

/* @source 0x801E1004
 * @behavior draws three panel decorations, a label, and optional numeric values.
 * @status partial
 * @match 80.10
 * @residual dynamic-label loop CFG and duplicate source-load scheduling differ
 */
typedef struct PackedCoords {
  u8 value[6];
} PackedCoords;

void func_801E1004(s32 arg0, s32 arg1, s32 arg2) {
  PackedCoords coords;
  s32 i;
  u8* dst;
  const u8* src;

  coords = *(PackedCoords*)D_801D0C30;
  submitTpageDrawMode(0, 1);
  for (i = 0; i < 3; i++) {
    s32 x;
    s32 y;
    if (i == 0) {
      x = coords.value[0];
      y = coords.value[1] - ((arg2 & 0xff) * 20);
    } else if (i == 1) {
      x = coords.value[2] - ((arg2 & 0xff) * 25);
      y = coords.value[3];
    } else {
      s32 offset = i << 1;
      const u8* pair = (const u8*)&coords + offset;
      x = pair[0] + ((arg2 & 0xff) * 25);
      y = pair[1];
    }
    func_801E0DCC((i + 3) & 0xff, 1, (s16)x, (s16)y);
  }

  dst = D_80145AD4;
  if ((arg0 & 0xff) == 0x16) {
    for (i = 0; i < 12; i++) *dst++ = D_801E28C4[i];
  } else if ((arg0 & 0xff) == 0xff) {
    for (i = 0; i < 12; i++) *dst++ = D_801E28D0[i];
  } else {
    src = D_801C8964 + (((arg0 & 0xff) + 0x38) * 18);
    i = 0;
    do {
      *dst = *src;
      i++;
      if (*src == 0) break;
      dst++;
      src++;
    } while (i < 12);
  }
  *dst = 0;
  func_8014F800(0x70, 0x45 - ((arg2 & 0xff) * 20), 0, 0xf, (u32)D_80145AD4);

  if ((arg0 & 0xff) != 0xff) {
    sprintf((char*)D_80145AD4, (const char*)D_801D0C38, arg1 & 0xff);
    for (i = 0; i < 3; i++) {
      if (D_80145AD4[i] == 0x20) D_80145AD4[i] = 0xff;
    }
    func_8014F800(0x2f - ((arg2 & 0xff) * 25), 0x73, 0, 3, (u32)D_80145AD4);
    sprintf((char*)D_80145AD4, (const char*)D_801D0C38,
            func_801E0ABC(arg0 & 0xff, arg1 & 0xff) & 0xffff);
    for (i = 0; i < 3; i++) {
      if (D_80145AD4[i] == 0x20) D_80145AD4[i] = 0xff;
    }
    func_8014F800(((arg2 & 0xff) * 25) + 0xef, 0x73, 0, 3,
                  (u32)D_80145AD4);
  }
}
