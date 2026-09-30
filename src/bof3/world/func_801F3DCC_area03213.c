#include "bof3/world/area03213_internal.h"

/* @source 0x801F3DCC
 * @behavior Copies the work record published at the scratchpad cursor
 *           0x1F800044 into the front-end record table at 0x80143FC8: stores
 *           the free-record index returned by func_8019601C into cursor byte
 *           0x0B and, when that byte is not 0xFF, marks the selected 0x74-byte
 *           record occupied (byte 0x00 set to 1), writes 0x50 into its byte
 *           0x05, copies cursor byte 0x08 into record byte 0x06, copies the
 *           cursor words at 0x34, 0x38 and 0x3C into the same record words
 *           adding 0x8000 to the 0x38 word and 0x1000000 to the 0x3C word, and
 *           writes 0x3C into record byte 0x09.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3DCC(void) {
  u8 index;
  u8* cursor;

  index = func_8019601C();
  D_1F800044[0xB] = index;
  if (D_1F800044[0xB] != 0xFF) {
    D_80143FC8[D_1F800044[0xB]].flags_00 = 1;
    D_80143FC8[D_1F800044[0xB]].unk_05 = 0x50;
    D_80143FC8[D_1F800044[0xB]].unk_06 = D_1F800044[8];
    cursor = D_1F800044;
    D_80143FC8[cursor[0xB]].unk_34 = *(s32*)(cursor + 0x34);
    D_80143FC8[cursor[0xB]].unk_38 = *(s32*)(cursor + 0x38) + 0x8000;
    D_80143FC8[cursor[0xB]].unk_3C = *(s32*)(cursor + 0x3C) + 0x1000000;
    D_80143FC8[cursor[0xB]].unk_09 = 0x3C;
  }
}
