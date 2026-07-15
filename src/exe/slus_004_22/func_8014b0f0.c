#include "internal.h"

extern u8   D_80145990[];
extern u32* D_801459D0[];

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
  ordering_table = D_801459D0;
  packet = D_80145990;
  work_offset = 0x70;
  do {
    AddPrims(D_80143E68 + work_offset,
             (void*)(((u32)D_80143D44 << 5) + (u32)packet), *ordering_table);
    ordering_table++;
    index++;
    packet += 4;
    work_offset += 4;
  } while (index < 8);
}
