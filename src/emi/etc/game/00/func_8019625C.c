#include "internal.h"

/* @behavior Advances active palette work records and writes their adjusted
 * colors into the shared palette buffer.
 * @source 0x8019625C
 */
void func_8019625C(void) {
  GamePaletteEntry* entry;
  s32               entry_index;
  s32               color_index;
  s32               count;
  u8                divisor;
  u8                quotient;
  u8                remainder;
  u8                scale;
  u8                source_index;
  u8                destination_index;
  u16*              source;
  u16*              destination;
  u16               color;
  s8                red;
  s8                green;
  s8                blue;
  u8                high_bit;

  for (entry_index = 0u; entry_index < 0x20u; entry_index++) {
    entry = &D_80145BD4[entry_index];
    if ((entry->flags & 1u) != 0u) {
      divisor = D_801C7AC8[entry->table_index];
      quotient = entry->field_01 / divisor;
      source_index = entry->field_01 % divisor;
      remainder = entry->step / divisor;
      destination_index = entry->step % divisor;
      scale = D_801C7AD0[entry->table_index];
      count = D_801C7AC0[entry->table_index] << 4;
      source_index = (u8)(scale * source_index);
      destination_index = (u8)(scale * destination_index);
      source = &D_80037800[quotient * 256u];
      destination = &D_80037800[remainder * 256u];

      for (color_index = 0; color_index < count; color_index++) {
        high_bit = 1u;
        if ((entry->flags & 2u) == 0u) {
          high_bit = (u8)(source[source_index] >> 15);
        }
        color = source[source_index];

        red = (s8)(color & 0x1fu);
        green = (s8)((color >> 5) & 0x1fu);
        blue = (s8)((color >> 10) & 0x1fu);

        red = (s8)(red + entry->red_offset);
        if (red < 0) {
          red = 0;
        }
        if (red >= 0x20) {
          red = 0x1f;
        }

        green = (s8)(green + entry->green_offset);
        if (green < 0) {
          green = 0;
        }
        if (green >= 0x20) {
          green = 0x1f;
        }

        blue = (s8)(blue + entry->blue_offset);
        if (blue < 0) {
          blue = 0;
        }
        if (blue >= 0x20) {
          blue = 0x1f;
        }

        if (color_index == 0u) {
          destination[destination_index] = 0u;
        } else {
          destination[destination_index + color_index] =
              (u16)((high_bit << 15) | ((u8)blue << 10) | ((u8)green << 5) |
                    (u8)red);
        }
        source_index++;
      }

      D_80145988 = 1u;
      if ((entry->flags & 0x40u) != 0u) {
        entry->flags &= 0xbfu;
        entry->target[0x27] = entry->step;
        if ((entry->flags & 0x80u) != 0u) {
          entry->target[0x24] |= 4u;
        }
      }
    }
  }
}
