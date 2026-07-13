#include "internal.h"

extern u8   DAT_80142cc0[];
extern u8   DAT_80142cc4[];
extern u8   DAT_80142ce0[];
extern u32  DAT_8014598c;
extern u8   DAT_80145990[];
extern u32* DAT_801459d0[];
extern u32  DAT_801459f8;
extern u32  DAT_801459fc;

/* @behavior rebuilds the active boot-frame ordering-table and packet-pointer
 * tables, then restores the two fixed packet arena bounds.
 * @source 0x8014b020 func_8014b020
 */
void func_8014b020(void) {
  s32   index;
  u32   buffer_index;
  u8*   packet;
  u8*   packet_table;
  u32** ordering_table;
  u32   table_offset;

  index = 0;
  ordering_table = DAT_801459d0;
  buffer_index = DAT_80143d44;
  packet_table = DAT_80145990;
  DAT_8014598c = 0x80020000u + ((buffer_index * 9) << 12);
  while (index < 8) {
    *ordering_table = (u32*)(packet_table + (buffer_index << 5));
    ordering_table++;
    index++;
    packet_table += 4;
  }

  index = 0;
  buffer_index = DAT_80143d44;
  packet = DAT_80142ce0;
  table_offset = (buffer_index + 4) << 3;
  do {
    *(u32*)(DAT_80142cc0 + table_offset) = 0;
    *(u32*)(DAT_80142cc4 + table_offset) = (u32)(packet + (buffer_index << 3));
    packet += 0x30;
    index++;
    table_offset += 0x30;
  } while (index < 0x38);

  DAT_801459f8 = 0x800e4800;
  DAT_801459fc = 0x800f5000;
}
