#include "bof3/world/area03004_internal.h"

/* @source 0x801E1004
 * @behavior draws three AREA030 panel decorations from the packed coordinate
 * record at D_801D0C30 (origin, plus the count-scaled 20/25-pixel offsets on
 * the first, second and fourth/fifth coordinate bytes), then copies the label
 * text into D_80145AD4 - the fixed 12-byte table D_801E28C4 for id 0x16, the
 * table D_801E28D0 for id 0xFF, otherwise the 18-byte row D_801C8964[id + 0x38]
 * up to its zero byte - terminates it and submits it at (0x70, 0x45 - 20 *
 * count) through func_8014F800; unless the id is 0xFF it then formats the byte
 * arg1 and the func_801E0ABC summary word through D_801D0C38 into the same
 * buffer (spaces rewritten to 0xFF) and submits them at x 0x2F - 25 * count and
 * 25 * count + 0xEF on row 0x73.
 * @status partial
 * @match 94.97
 * @residual the third-arm copy loop's second read is missing: the current
 * object reuses the one `lbu 0($v1)` load for the zero test and puts the
 * `sb 0($t0)` store in that branch's delay slot, so the original's second
 * `lbu 0(v1)` reload is absent and the loop-exit and later labels sit eight
 * bytes (two instructions) earlier; every other instruction, the packed-record
 * copy, the three draw calls without a short conversion on the count-scaled
 * coordinates, and the trailing label formatting match
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
    i = 0;
    src = D_801C8964 + (((arg0 & 0xff) + 0x38) * 18);
    while (i < 12) {
      *dst = *src;
      if (*src == 0) {
        i++;
        break;
      }
      i++;
      dst++;
      src++;
    }
  }
  *dst = 0;
  func_8014F800(0x70, 0x45 - ((u8)arg2 * 20), 0, 0xf, (u32)D_80145AD4);

  if ((arg0 & 0xff) != 0xff) {
    sprintf((char*)D_80145AD4, (const char*)D_801D0C38, arg1 & 0xff);
    for (i = 0; i < 3; i++) {
      if (D_80145AD4[i] == 0x20) D_80145AD4[i] = 0xff;
    }
    func_8014F800(0x2f - ((u8)arg2 * 25), 0x73, 0, 3, (u32)D_80145AD4);
    sprintf((char*)D_80145AD4, (const char*)D_801D0C38,
            func_801E0ABC(arg0 & 0xff, arg1 & 0xff) & 0xffff);
    for (i = 0; i < 3; i++) {
      if (D_80145AD4[i] == 0x20) D_80145AD4[i] = 0xff;
    }
    func_8014F800(((u8)arg2 * 25) + 0xef, 0x73, 0, 3,
                  (u32)D_80145AD4);
  }
}
