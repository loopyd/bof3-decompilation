#include "bof3/battle/battle03_internal.h"

/* @source 0x801E9B18
 * @behavior Dispatches the panel-task state handler selected by panel byte +3 through
 * the three-entry handler table copied off 0x801D0FE0, forwards the panel-task field
 * pointers +0x8/+0x9, +0xB/+0xC, +0xD/+0xE, +0x14/+0x16, +0x18/+0x1A and +0x1C/+0x1E
 * together with the selected 0x140-stride local work record's pointers +0x88/+0x8A and
 * +0x90/+0x92 to func_801EA1E0 once per panel hand, then raises bit 0x2000 of that
 * record's flags halfword +0x80 while its halfword +0x88 is below a quarter of its
 * halfword +0x90 and clears the bit otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E9B18(void) {
  Battle03DispatchTable local_20;
  u8*                   work_base;
  u8*                   record;
  u32                   index;
  u32                   offset;

  local_20 = D_801D0FE0;
  local_20.handlers[D_80148648[3]]();

  work_base = (u8*)D_80145E90 + 0x74;
  index = D_80148648[0xA];
  record = work_base + index * 0x140;
  func_801EA1E0((s32)(D_80148648 + 0xB), (s32)(record + 0x1C),
                (s32)(D_80148648 + 0x1C), (s32)(D_80148648 + 0x14),
                (s32)(record + 0x14), (s32)(D_80148648 + 0xD),
                (s32)(D_80148648 + 0x18), D_80148648 + 0x8);

  index = D_80148648[0xA];
  record = work_base + index * 0x140;
  func_801EA1E0((s32)(D_80148648 + 0xC), (s32)(record + 0x1E),
                (s32)(D_80148648 + 0x1E), (s32)(D_80148648 + 0x16),
                (s32)(record + 0x16), (s32)(D_80148648 + 0xE),
                (s32)(D_80148648 + 0x1A), D_80148648 + 0x9);

  offset = (((u32)D_80148648[0xA] << 2) + D_80148648[0xA]) << 6;
  if (*(volatile u16*)&D_80145F18[offset] <
      (*(volatile u16*)&D_80145F20[offset] >> 2)) {
    *(volatile u16*)&D_80145F10[offset] |= 0x2000u;
  } else {
    *(volatile u16*)&D_80145F10[offset] &= 0xdfffu;
  }
}
