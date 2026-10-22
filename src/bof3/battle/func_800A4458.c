#include "bof3/battle/battle15_internal.h"

/* @behavior Re-derives the screen position and colour band of the three local
 *   party slots from the shared template at 0x801EBEF0. For every slot whose
 *   flag word at 0x80145FB8 + slot*0x140 has bit 0x2 set, the template's
 *   leading halfword becomes the slot's x position scaled down by the slot's
 *   weight byte at 0x80145F1E + slot*0x140, the following four template
 *   halfwords offset the slot's four y/offset halfwords, and the nine template
 *   bytes 0x0A..0x12 are copied into the colour band. Each result is written
 *   both to the 0x140-byte party record at 0x80145E90 + slot*0x140 and to its
 *   parallel band at 0x80145F20 + slot*0x140.
 * @source 0x800A4458
 * @status partial
 * @match 85.00
 * @residual allocation: 119/140 instructions and an exact 560-byte image; the
 *   party record base lands in $a1 where the original holds it in $a0 and the
 *   scaled template halfword in $a1, a two-register swap across the whole loop
 *   body, accounting for 21 register substitutions. The other instructions
 *   and both store targets match.
 */
void func_800A4458(void) {
  u8* tmpl;
  u8* record;
  u32 offset;
  u8 slot;
  s16 x;
  u16 value;
  u8 colour;

  slot = 0u;
  tmpl = D_801EBEF0;
  do {
    offset = (u32)slot * 0x140u;
    if ((D_80145FB8[slot].flags_00 & 0x2u) != 0u) {
      x = *(s16*)&tmpl[0];
      x = x - (x * D_80145F1E[offset] + 5) / 10;
      record = (u8*)D_80145E90;
      record += offset;

      *(u16*)&record[0xB0] = x;
      value = *(u16*)&D_80145F44[offset];
      *(u16*)&D_80145F20[offset] = x;
      value += *(u16*)&tmpl[2];
      *(u16*)&record[0xB4] = value;
      *(u16*)&D_80145F24[offset] = value;

      value = *(u16*)&D_80145F46[offset];
      value += *(u16*)&tmpl[4];
      *(u16*)&record[0xB6] = value;
      *(u16*)&D_80145F26[offset] = value;

      value = *(u16*)&D_80145F48[offset];
      value += *(u16*)&tmpl[6];
      *(u16*)&record[0xB8] = value;
      *(u16*)&D_80145F28[offset] = value;

      value = *(u16*)&D_80145F4A[offset];
      value += *(u16*)&tmpl[8];
      *(u16*)&record[0xBA] = value;
      *(u16*)&D_80145F2A[offset] = value;

      colour = tmpl[0x0A];
      record[0xBF] = colour;
      D_80145F2F[offset] = colour;
      colour = tmpl[0x0B];
      record[0xC0] = colour;
      D_80145F30[offset] = colour;
      colour = tmpl[0x0C];
      record[0xC1] = colour;
      D_80145F31[offset] = colour;
      colour = tmpl[0x0D];
      record[0xC2] = colour;
      D_80145F32[offset] = colour;
      colour = tmpl[0x0E];
      record[0xC3] = colour;
      D_80145F33[offset] = colour;
      colour = tmpl[0x0F];
      record[0xC4] = colour;
      D_80145F34[offset] = colour;
      colour = tmpl[0x10];
      record[0xC5] = colour;
      D_80145F35[offset] = colour;
      colour = tmpl[0x11];
      record[0xC6] = colour;
      D_80145F36[offset] = colour;
      colour = tmpl[0x12];
      record[0xC7] = colour;
      D_80145F37[offset] = colour;
    }
    slot++;
  } while (slot < 3u);
}
