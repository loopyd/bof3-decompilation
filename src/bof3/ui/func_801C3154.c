#include "bof3/ui/game00_internal.h"

/* @behavior When bit 3 of the active work record's flags byte at 0x12C is set,
 * copies the 31 palette words of the CLUT source block selected by the scratch
 * work byte at 0x27 from 0x80033800 into the VRAM shadow block at 0x80037800,
 * advances the palette stage serial, and clears that bit.
 * @source 0x801C3154
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit is instruction- and byte-exact: 44/44 instructions, 176 bytes.
 * The split base form (`row`, then the destination base add, then the sub-block
 * offset) placed the first difference at +0x005C; re-deriving the destination
 * from the base plus sub plus row closed the last `addu` operand order.
 */
void func_801C3154(void) {
  s32   i;
  u8    index;
  u32   row;
  u32   sub;
  u16*  src;
  u16*  dst;
  u16   value;
  u8*   work;

  if ((D_80146250[0x12C] & 8u) == 0u) {
    return;
  }

  i = 1;
  do {
    index = ((u8*)g_game_work)[0x27];
    row = (u32)(index >> 3) * 0x200u;
    dst = PSX_PTR(u16, 0x80037800u + row);
    sub = (u32)(index & 7u) * 0x40u;
    dst = PSX_PTR(u16, 0x80037800u + sub + row);
    src = PSX_PTR(u16, 0x80033800u + sub + row);
    value = src[i];
    dst[i] = value;
    i++;
  } while (i < 0x20);

  work = D_80146250;
  paletteStageSerial = 1u;
  work[0x12C] &= 0xF7u;
}
