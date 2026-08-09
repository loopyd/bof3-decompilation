#include "bof3/battle/battle15_internal.h"

/* @behavior handles battle camera adjustment during the selection phase. depending
 * on the current mode (1=defend, 2=attack, 3=examine, 4=escape), adjusts
 * camera-position variables and resets per-character visual offsets.
 * @source 0x800AAA74
 * @status partial
 * @match 36.88
 * @residual non-exact live audit: 59/133 instructions; 532 original bytes versus 640 current.
 */
void func_800AAA74(void) {
  volatile u8*  state_base;
  volatile u16* panel_offset;
  u8            mode;
  u8            slot;
  u8            active_count;
  s32           slot_offset;

  state_base = BATTLE_GAME_RAM_BASE;
  mode = PSX_REF(volatile u8, 0x80144f58u);

  switch (mode) {
    case 1u:
      FUNCTION_AT(void (*)(volatile u16*, u32), 0x801654f4u)(
          BATTLE_UNK_80145F44, (u32)(PSX_REF(volatile u16, 0x80145f44u) >> 1u));
      FUNCTION_AT(void (*)(volatile u16*, s32), 0x801654f4u)(
          BATTLE_UNK_80145F46, -(s32)(PSX_REF(volatile u16, 0x80145f46u) >> 2u));
      FUNCTION_AT(void (*)(volatile u8*, u8), 0x80165694u)(BATTLE_UNK_80145F59,
                                                           PSX_REF(volatile u8, 0x80145f59u));
      return;
    case 2u:
      active_count = PSX_REF(volatile u8, 0x801462f0u);
      slot = 0u;
      while (slot < active_count) {
        if (!FUNCTION_AT(s32 (*)(s32), 0x801db524u)((s32)slot)) {
          slot_offset = (u32)slot * 0x140u;
          panel_offset = (volatile u16*)(state_base + slot_offset + 0x5f04u);
          FUNCTION_AT(void (*)(volatile u16*, s32), 0x801654f4u)(
              panel_offset + (0x44u / 2u),
              -(s32)(PSX_REF(volatile u16, 0x80145f48u + slot_offset) >> 2u));
        }
        slot += 1u;
      }
      return;
    case 3u:
      active_count = PSX_REF(volatile u8, 0x801462f0u);
      slot = 0u;
      while (slot < active_count) {
        if (!FUNCTION_AT(s32 (*)(s32), 0x801db524u)((s32)slot)) {
          slot_offset = (u32)slot * 0x140u;
          PSX_REF(volatile u16, 0x80145f48u + slot_offset) = PSX_REF(volatile u16, 0x80145f48u);
          panel_offset = (volatile u16*)(state_base + slot_offset + 0x5f04u);
          FUNCTION_AT(void (*)(volatile u16*, s32), 0x801654f4u)(
              panel_offset + (0x42u / 2u),
              -(s32)(PSX_REF(volatile u16, 0x80145f46u + slot_offset) >> 1u));
        }
        slot += 1u;
      }
      break;
    case 4u:
      FUNCTION_AT(void (*)(volatile u16*, u32), 0x801654f4u)(
          BATTLE_UNK_801461CA, (u32)(PSX_REF(volatile u16, 0x801461cau) >> 1u));
      FUNCTION_AT(void (*)(volatile u16*, s32), 0x801654f4u)(
          BATTLE_UNK_80145F4A, -(s32)(PSX_REF(volatile u16, 0x80145f4au) >> 2u));
      FUNCTION_AT(void (*)(volatile u16*, s32), 0x801654f4u)(
          BATTLE_UNK_80145F44, -(s32)(PSX_REF(volatile u16, 0x80145f44u) >> 2u));
      break;
  }
}
