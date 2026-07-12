#include "internal.h"

/* @behavior handles battle camera adjustment during the selection phase. depending
 * on the current mode (1=defend, 2=attack, 3=examine, 4=escape), adjusts
 * camera-position variables and resets per-character visual offsets.
 * @source 0x800aaa74 FUN_800aaa74
 */
void func_800aaa74(void) {
  volatile u8*  state_base;
  volatile u16* panel_offset;
  u8            mode;
  u8            slot;
  u8            active_count;
  s32           slot_offset;

  state_base = (volatile u8*)0x80140000u;
  mode = REG8(0x80144f58u);

  switch (mode) {
    case 1u:
      ((void (*)(volatile u16*, u32))0x801654f4u)(
          (volatile u16*)0x80145f44u, (u32)(REG16(0x80145f44u) >> 1u));
      ((void (*)(volatile u16*, s32))0x801654f4u)(
          (volatile u16*)0x80145f46u, -(s32)(REG16(0x80145f46u) >> 2u));
      ((void (*)(volatile u8*, u8))0x80165694u)((volatile u8*)0x80145f59u,
                                                REG8(0x80145f59u));
      return;
    case 2u:
      active_count = REG8(0x801462f0u);
      slot = 0u;
      while (slot < active_count) {
        if (!((s32 (*)(s32))0x801db524u)((s32)slot)) {
          slot_offset = (u32)slot * 0x140u;
          panel_offset = (volatile u16*)(state_base + slot_offset + 0x5f04u);
          ((void (*)(volatile u16*, s32))0x801654f4u)(
              panel_offset + (0x44u / 2u),
              -(s32)(REG16(0x80145f48u + slot_offset) >> 2u));
        }
        slot += 1u;
      }
      return;
    case 3u:
      active_count = REG8(0x801462f0u);
      slot = 0u;
      while (slot < active_count) {
        if (!((s32 (*)(s32))0x801db524u)((s32)slot)) {
          slot_offset = (u32)slot * 0x140u;
          REG16(0x80145f48u + slot_offset) = REG16(0x80145f48u);
          panel_offset = (volatile u16*)(state_base + slot_offset + 0x5f04u);
          ((void (*)(volatile u16*, s32))0x801654f4u)(
              panel_offset + (0x42u / 2u),
              -(s32)(REG16(0x80145f46u + slot_offset) >> 1u));
        }
        slot += 1u;
      }
      break;
    case 4u:
      ((void (*)(volatile u16*, u32))0x801654f4u)(
          (volatile u16*)0x801461cau, (u32)(REG16(0x801461cau) >> 1u));
      ((void (*)(volatile u16*, s32))0x801654f4u)(
          (volatile u16*)0x80145f4au, -(s32)(REG16(0x80145f4au) >> 2u));
      ((void (*)(volatile u16*, s32))0x801654f4u)(
          (volatile u16*)0x80145f44u, -(s32)(REG16(0x80145f44u) >> 2u));
      break;
  }
}
