#include "bof3/core/slus_internal.h"

extern u8   D_80142CC0[];
extern u8   D_80142CC4[];
extern u8   D_80142CE0[];
extern u32  g_PrimCursor;
extern u8   bootPacketHeadTable[];
extern u32* bootOrderingTableHeads[];
extern u32  D_801459F8;
extern u32  D_801459FC;

/* @behavior rebuilds the active boot-frame ordering-table and packet-pointer
 * tables, then restores the two fixed packet arena bounds.
 * @source 0x8014B020
 * @status partial
 * @match 42.31
 * @residual loop induction shape only: 22/52 instructions; 208 original bytes
 * versus 204 current. Both loops compare an ascending signed counter against a
 * literal bound, so the canonical -O2 profile rewrites each into a countdown
 * trip counter and schedules the address materialisation ahead of the counter.
 */
void rebuildBootRenderTables(void) {
  s32   index;
  u32   buffer_index;
  u8*   packet;
  u8*   packet_table;
  u32** ordering_table;
  u32   table_offset;

  index = 0;
  ordering_table = bootOrderingTableHeads;
  buffer_index = D_80143D44;
  packet_table = bootPacketHeadTable;
  g_PrimCursor = 0x80020000u + ((buffer_index * 9) << 12);
  do {
    *ordering_table = (u32*)(packet_table + (buffer_index << 5));
    ordering_table++;
    packet_table += 4;
  } while (++index < 8);

  index = 0;
  buffer_index = D_80143D44;
  packet = D_80142CE0;
  table_offset = (buffer_index + 4) << 3;
  do {
    *(u32*)(D_80142CC0 + table_offset) = 0;
    *(u32*)(D_80142CC4 + table_offset) = (u32)(packet + (buffer_index << 3));
    packet += 0x30;
    table_offset += 0x30;
  } while (++index < 0x38);

  D_801459F8 = 0x800e4800;
  D_801459FC = 0x800f5000;
}
