#include "internal.h"

/* does: applies a bitmask of status/mode flags to clear corresponding bits in
 * a battler's status word. for the magic-defence bit (0x20), copies three
 * defence values from a shared table into the battler's slot.
 * returns the updated status word.
 * @source: 0x800a36f0 FUN_800a36f0
 */
u16 func_800a36f0(u8 battler_index, u16 flags) {
  volatile u8*  player_base;
  volatile u16* status_slot;
  u16           status;
  u32           scratchpad_saved;
  u8            player;
  u8            enemy;

  player_base = (volatile u8*)0x80140000u;

  if (battler_index < 3u) {
    status = REG16(0x80145f10u + ((u32)battler_index * 0x140u));
  } else {
    status = REG16(0x801eb6b2u + ((u32)(battler_index - 3u) * 0x118u));
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
      REG32(0x80145ec4u + ((u32)battler_index * 0x140u)) =
          REG32(0x801ec364u + ((u32)battler_index * 0x78u));
      REG32(0x80145ec8u + ((u32)battler_index * 0x140u)) =
          REG32(0x801ec368u + ((u32)battler_index * 0x78u));
      REG32(0x80145eccu + ((u32)battler_index * 0x140u)) =
          REG32(0x801ec36cu + ((u32)battler_index * 0x78u));
    } else {
      enemy = battler_index - 3u;
      REG32(0x801eb664u + ((u32)enemy * 0x118u)) =
          REG32(0x801ec364u + ((u32)battler_index * 0x78u));
      REG32(0x801eb668u + ((u32)enemy * 0x118u)) =
          REG32(0x801ec368u + ((u32)battler_index * 0x78u));
      REG32(0x801eb66cu + ((u32)enemy * 0x118u)) =
          REG32(0x801ec36cu + ((u32)battler_index * 0x78u));
    }
  }

  if (battler_index < 3u) {
    player = battler_index;
    ((void (*)(volatile u8*))0x80196718u)(player_base + ((u32)player * 0x140u) +
                                          0x5e90u);
    scratchpad_saved = REG32(0x1f800044u);
    REG16(0x80145f10u + ((u32)player * 0x140u)) = status;
    REG32(0x1f800044u) = (u32)(player_base + ((u32)player * 0x140u) + 0x5e90u);
  } else {
    enemy = battler_index - 3u;
    ((void (*)(volatile u8*))0x80196718u)(
        (volatile u8*)(0x801eb2e8u + ((u32)enemy * 0x118u)));
    scratchpad_saved = REG32(0x1f800044u);
    REG16(0x801eb6b2u + ((u32)enemy * 0x118u)) = status;
    REG32(0x1f800044u) = (u32)(0x801eb2e8u + ((u32)enemy * 0x118u));
  }

  ((void (*)(u16))0x801ddab4u)(status);
  REG32(0x1f800044u) = scratchpad_saved;

  return status;
}
