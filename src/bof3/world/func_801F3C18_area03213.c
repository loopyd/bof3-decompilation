#include "bof3/world/area03213_internal.h"

/* @source 0x801F3C18
 * @behavior Stores the free-record index returned by func_8019601C into byte
 *           0x0B of the current work record published at the scratchpad cursor
 *           0x1F800044 and, when that byte is not 0xFF, marks the front-end
 *           record it selects at 0x80143FC8 as occupied (record byte 0x00 set
 *           to 1), writes 0x4F into the record byte 0x05, and stores the index
 *           of the current work record (the 0x80146884 minus 0x80146888
 *           pointer difference) in the record byte 0x0B.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3C18(void) {
  u8 index;

  index = func_8019601C();
  D_1F800044[0xB] = index;
  if (D_1F800044[0xB] != 0xFF) {
    D_80143FC8[D_1F800044[0xB]].flags_00 = 1;
    D_80143FC8[D_1F800044[0xB]].unk_05 = 0x4F;
    D_80143FC8[D_1F800044[0xB]].unk_0B =
        (u8)((World00Area032WorkRecord*)D_80146884 -
             (World00Area032WorkRecord*)D_80146888);
  }
}
