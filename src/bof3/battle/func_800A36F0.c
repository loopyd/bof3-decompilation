#include "bof3/battle/battle15_internal.h"

/* @behavior applies a bitmask of status/mode flags to clear corresponding bits in
 * a battler's status word. for the magic-defence bit (0x20), copies three
 * defence values from a shared table into the battler's slot.
 * returns the updated status word.
 * @source 0x800A36F0
 * @status partial
 * @match 15.25
 * @residual non-exact live audit: 27/177 instructions; 708 original bytes versus 708 current.
 */
u16 func_800A36F0(u8 battler_index, u16 flags) {
  volatile u8*  player_base;
  volatile u16* status_slot;
  u16           status;
  u16           applied_status;
  u8*           scratchpad_saved;
  u8            player;
  u8            enemy;

  player_base = BATTLE_GAME_RAM_BASE;

  if (battler_index < 3u) {
    status = PSX_REF(volatile u16, 0x80145f10u + ((u32)battler_index * 0x140u));
  } else {
    status = PSX_REF(volatile u16, 0x801eb6b2u + ((u32)(battler_index - 3u) * 0x118u));
  }

  if (flags & 0x4000u) {
    status &= 0xbfffu;
  }
  if (flags & 0x80u) {
    status &= 0xff7fu;
  }
  if (flags & 0x1u) {
    status &= 0xfffeu;
  }
  if (flags & 0x2u) {
    status &= 0xfffdu;
  }
  if (flags & 0x4u) {
    status &= 0xfffbu;
  }
  if (flags & 0x40u) {
    status &= 0xffbfu;
  }
  if (flags & 0x8u) {
    status &= 0xfff7u;
  }
  if (flags & 0x10u) {
    status &= 0xffefu;
  }
  if (flags & 0x800u) {
    status &= 0xf7ffu;
  }

  if ((flags & 0x20u) && (status & 0x20u)) {
    status &= 0xffdfu;

    if (battler_index < 3u) {
      PSX_REF(volatile u32, 0x80145ec4u + ((u32)battler_index * 0x140u)) =
          PSX_REF(volatile u32, 0x801ec364u + ((u32)battler_index * 0x78u));
      PSX_REF(volatile u32, 0x80145ec8u + ((u32)battler_index * 0x140u)) =
          PSX_REF(volatile u32, 0x801ec368u + ((u32)battler_index * 0x78u));
      PSX_REF(volatile u32, 0x80145eccu + ((u32)battler_index * 0x140u)) =
          PSX_REF(volatile u32, 0x801ec36cu + ((u32)battler_index * 0x78u));
    } else {
      enemy = battler_index - 3u;
      PSX_REF(volatile u32, 0x801eb664u + ((u32)enemy * 0x118u)) =
          PSX_REF(volatile u32, 0x801ec364u + ((u32)battler_index * 0x78u));
      PSX_REF(volatile u32, 0x801eb668u + ((u32)enemy * 0x118u)) =
          PSX_REF(volatile u32, 0x801ec368u + ((u32)battler_index * 0x78u));
      PSX_REF(volatile u32, 0x801eb66cu + ((u32)enemy * 0x118u)) =
          PSX_REF(volatile u32, 0x801ec36cu + ((u32)battler_index * 0x78u));
    }
  }

  if (battler_index < 3u) {
    player = battler_index;
    FUNCTION_AT(void (*)(volatile u8*),
                0x80196718u)(player_base + ((u32)player * 0x140u) + 0x5e90u);
    scratchpad_saved = g_battle_work;
    status_slot = PSX_PTR(volatile u16, 0x80145f10u + ((u32)player * 0x140u));
    *status_slot = status;
    applied_status = *status_slot;
    g_battle_work = (u8*)(player_base + ((u32)player * 0x140u) + 0x5e90u);
  } else {
    enemy = battler_index - 3u;
    FUNCTION_AT(void (*)(volatile u8*), 0x80196718u)(
        BATTLE_ENEMY_BATTLER_BASE + ((u32)battler_index * 0x118u));
    scratchpad_saved = g_battle_work;
    status_slot = PSX_PTR(volatile u16, 0x801eb6b2u + ((u32)enemy * 0x118u));
    *status_slot = status;
    applied_status = *status_slot;
    g_battle_work = (u8*)(BATTLE_ENEMY_BATTLER_BASE + ((u32)battler_index * 0x118u));
  }

  FUNCTION_AT(void (*)(u16), 0x801ddab4u)(applied_status);
  g_battle_work = scratchpad_saved;

  return status;
}
