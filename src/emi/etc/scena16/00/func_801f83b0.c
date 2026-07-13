#include "internal.h"

/* @behavior copies one palette window while clamping each component to one limit.
 * @source 0x801f83b0 FUN_801f83b0
 */
void func_801f83b0(u32 intensity) {
  const u16* src;
  const u16* src_end;
  u32        dst_offset;
  u32        limit;

  intensity &= 0xffu;
  limit = intensity & 0xffffu;
  dst_offset = 0u;
  src = (const u16*)SCENA16_PALETTE_SRC;
  src_end = (const u16*)0x80033820u;

  do {
    u16  color;
    u32  shift;
    s32  component_index;
    u32  packed;
    u32  component;
    u32  next_packed;
    u16* dst;

    color = *src;
    shift = 0u;
    component_index = 0;
    packed = 0u;
    component = 0u;
    next_packed = 0u;

    do {
      component = ((u32)color >> shift) & 0x1fu;
      next_packed = packed << 5;
      if (limit < component) {
        component = intensity;
      }
      packed = next_packed | component;
      component_index++;
      shift += 5u;
    } while (component_index < 3);

    dst = (u16*)(0x80037800u + dst_offset);
    *dst = (u16)packed;
    if ((next_packed & 0xffffu) != 0u || component != 0u) {
      *dst = (u16)(packed | 0x8000u);
    }

    src++;
    dst_offset += 2u;
  } while (src < src_end);

  SCENA16_DAT_80145988 = (u8)(SCENA16_DAT_80145988 + 1u);
}
