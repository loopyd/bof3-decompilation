#include "internal.h"

extern u8   DAT_80145990[];
extern u32* DAT_801459d0[];

/* @behavior links the active frame's eight ordering-table heads to their
 * packet chains with the PsyQ AddPrims helper.
 * @source 0x8014b0f0 func_8014b0f0
 */
void func_8014b0f0(void) {
  s32   index;
  u32** ordering_table;
  u8*   packet;
  s32   work_offset;

  index = 0;
  ordering_table = DAT_801459d0;
  packet = DAT_80145990;
  work_offset = 0x70;
  do {
    AddPrims(DAT_80143e68 + work_offset,
             (void*)(((u32)DAT_80143d44 << 5) + (u32)packet),
             *ordering_table);
    ordering_table++;
    index++;
    packet += 4;
    work_offset += 4;
  } while (index < 8);
}
