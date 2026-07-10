#include "internal.h"

/* does: initialises battle stat display fields for the first three player
 * slots from a shared template at 0x801ebef0. copies screen-position
 * coordinates, element-resistance indices, and palette/colour values.
 * @source: 0x800a4458 FUN_800a4458
 */
void func_800a4458(void) {
  volatile s16* template_s16;
  volatile u8*  template_u8;
  volatile u8*  player_base;
  volatile u8*  player_slot;
  u32           slot_offset;
  u8            slot;
  s16           base_x;
  s16           base_x_adj;

  template_s16 = (volatile s16*)0x801ebef0u;
  template_u8 = (volatile u8*)0x801ebef0u;
  player_base = (volatile u8*)0x80140000u;

  slot = 0u;
  do {
    slot_offset = (u32)slot * 0x140u;

    if (REG32(0x80145fb4u + slot_offset + 4u) & 0x2u) {
      base_x = template_s16[0];
      base_x_adj =
          (base_x -
           (((base_x * (s32)REG8(0x80145f1eu + slot_offset)) + 5) / 10));

      REG16((u32)(player_base + slot_offset + 0x5e90u + 0xb0u)) = base_x_adj;
      REG16(0x80145f20u + slot_offset) = base_x_adj;

      REG16((u32)(player_base + slot_offset + 0x5e90u + 0xb4u)) =
          (s16)((u16)REG16(0x80145f44u + slot_offset) + (u16)template_s16[1]);
      REG16(0x80145f24u + slot_offset) =
          (s16)((u16)REG16(0x80145f44u + slot_offset) + (u16)template_s16[1]);

      REG16((u32)(player_base + slot_offset + 0x5e90u + 0xb6u)) =
          (s16)((u16)REG16(0x80145f46u + slot_offset) + (u16)template_s16[2]);
      REG16(0x80145f26u + slot_offset) =
          (s16)((u16)REG16(0x80145f46u + slot_offset) + (u16)template_s16[2]);

      REG16((u32)(player_base + slot_offset + 0x5e90u + 0xb8u)) =
          (s16)((u16)REG16(0x80145f48u + slot_offset) + (u16)template_s16[3]);
      REG16(0x80145f28u + slot_offset) =
          (s16)((u16)REG16(0x80145f48u + slot_offset) + (u16)template_s16[3]);

      REG16((u32)(player_base + slot_offset + 0x5e90u + 0xbau)) =
          (s16)((u16)REG16(0x80145f4au + slot_offset) + (u16)template_s16[4]);
      REG16(0x80145f2au + slot_offset) =
          (s16)((u16)REG16(0x80145f4au + slot_offset) + (u16)template_s16[4]);

      REG8((u32)(player_base + slot_offset + 0x5e90u + 0xbfu)) =
          template_u8[0xau];
      REG8(0x80145f2fu + slot_offset) = template_u8[0xau];
      REG8((u32)(player_base + slot_offset + 0x5e90u + 0xc0u)) =
          template_u8[0xbu];
      REG8(0x80145f30u + slot_offset) = template_u8[0xbu];
      REG8((u32)(player_base + slot_offset + 0x5e90u + 0xc1u)) =
          template_u8[0xcu];
      REG8(0x80145f31u + slot_offset) = template_u8[0xcu];
      REG8((u32)(player_base + slot_offset + 0x5e90u + 0xc2u)) =
          template_u8[0xdu];
      REG8(0x80145f32u + slot_offset) = template_u8[0xdu];
      REG8((u32)(player_base + slot_offset + 0x5e90u + 0xc3u)) =
          template_u8[0xeu];
      REG8(0x80145f33u + slot_offset) = template_u8[0xeu];
      REG8((u32)(player_base + slot_offset + 0x5e90u + 0xc4u)) =
          template_u8[0xfu];
      REG8(0x80145f34u + slot_offset) = template_u8[0xfu];
      REG8((u32)(player_base + slot_offset + 0x5e90u + 0xc5u)) =
          template_u8[0x10u];
      REG8(0x80145f35u + slot_offset) = template_u8[0x10u];
      REG8((u32)(player_base + slot_offset + 0x5e90u + 0xc6u)) =
          template_u8[0x11u];
      REG8(0x80145f36u + slot_offset) = template_u8[0x11u];
      REG8((u32)(player_base + slot_offset + 0x5e90u + 0xc7u)) =
          template_u8[0x12u];
      REG8(0x80145f37u + slot_offset) = template_u8[0x12u];
    }

    slot += 1u;
  } while (slot < 3u);
}
