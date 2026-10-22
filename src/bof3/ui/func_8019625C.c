#include "bof3/ui/game00_internal.h"

/* @behavior Advances active palette work records and writes their adjusted
 * colors into the shared palette buffer.
 * @source 0x8019625C
 * @status partial
 * @match 60.92
 * @residual Non-exact live audit: 106/174 instructions, 704 original bytes
 * versus 668 current. Six of the nine missing instructions are the two
 * three-instruction divisor-zero guards (bnez v0; nop; break 7) that psyq
 * maspsx emits only under -Wa,--expand-div; that object entry
 * (BOF3_OBJFLAGS_bof3_ui_func_8019625C_c) is parent-owned and is not applied
 * here. The remaining three are allocator/scheduling class: the divide and
 * mult web keeps the table index in $a1 and the field value in $a0 where the
 * original has the opposite pairing, so the quotient needs one extra copy
 * before the 512-byte palette-row scale, and each channel clamp keeps its own
 * branch-block order - the original reaches the zero arm through bltz with
 * the clamp-high j/li block out of line, while this build inlines the zero arm
 * on the bgez path. The shared palette base is reached
 * through the reviewable fixed-address g_PaletteDst view, which reproduces the
 * original's lui/ori constant load; the 16-bit palette reads stay volatile
 * because the original loads the same source colour twice.
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
  volatile u16*     source;
  volatile u16*     destination;
  u16               color;
  s8                red;
  s8                green;
  s8                blue;
  u8                high_bit;
  u8                value;
  s8                sv;

  for (entry_index = 0; entry_index < 0x20; entry_index++) {
    entry = &D_80145BD4[entry_index];
    if ((entry->flags & 1) != 0) {
      divisor = D_801C7AC8[entry->table_index];
      quotient = entry->field_01 / divisor;
      source_index = entry->field_01 % divisor;
      remainder = entry->step / divisor;
      destination_index = entry->step % divisor;
      scale = D_801C7AD0[entry->table_index];
      count = D_801C7AC0[entry->table_index] << 4;
      source_index = (u8)(scale * source_index);
      destination_index = (u8)(scale * destination_index);
      source = &g_PaletteDst[quotient * 256u];
      destination = &g_PaletteDst[remainder * 256u];

      for (color_index = 0; color_index < count; color_index++) {
        high_bit = 1u;
        if ((entry->flags & 2u) == 0u) {
          high_bit = (u8)(source[source_index + color_index] >> 15);
        }
        color = source[source_index + color_index];

        red = (s8)(color & 0x1fu);
        blue = (s8)((color >> 10) & 0x1fu);
        green = (s8)((color >> 5) & 0x1fu);

        value = (u8)(red + entry->red_offset);
        sv = (s8)value;
        if (sv < 0) {
          red = 0;
        } else if (sv < 0x20) {
          red = value;
        } else {
          red = 0x1f;
        }

        value = (u8)(green + entry->green_offset);
        sv = (s8)value;
        if (sv < 0) {
          green = 0;
        } else if (sv < 0x20) {
          green = value;
        } else {
          green = 0x1f;
        }

        value = (u8)(blue + entry->blue_offset);
        sv = (s8)value;
        if (sv < 0) {
          blue = 0;
        } else if (sv < 0x20) {
          blue = value;
        } else {
          blue = 0x1f;
        }

        if (color_index == 0u) {
          destination[destination_index] = 0u;
        } else {
          destination[destination_index + color_index] =
              (high_bit << 15) | (blue << 10) | (green << 5) | red;
        }
      }

      paletteStageSerial = 1u;
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
