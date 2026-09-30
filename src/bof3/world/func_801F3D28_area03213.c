#include "bof3/world/area03213_internal.h"

/* @source 0x801F3D28
 * @behavior Stores the free-record index returned by func_8019601C into byte
 *           0x0B of the current work record published at the scratchpad cursor
 *           0x1F800044 and, when that byte is not 0xFF, marks the front-end
 *           record it selects at 0x80143FC8 as occupied (record byte 0x00 set
 *           to 1) and writes 0x4E into the record byte 0x05.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3D28(void) {
  u8 index;

  index = func_8019601C();
  D_1F800044[0xB] = index;
  if (D_1F800044[0xB] != 0xFF) {
    D_80143FC8[D_1F800044[0xB]].flags_00 = 1;
    D_80143FC8[D_1F800044[0xB]].unk_05 = 0x4E;
  }
}
