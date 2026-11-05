#include "bof3/world/area03004_internal.h"

/* @source 0x801D57B0
 * @behavior AREA030 panel-step initializer (handler index 0 of the work-byte-3
 * table at 0x801E1FD4): twice it claims the front-end entry record that the
 * shared allocator func_8019601c reports, skipping the claim when that index is
 * the 0xFF sentinel; each claimed record gets byte 0 = 1, byte 5 = 0x1A,
 * byte 1 = 0x10 and byte 3 = 2 for the first claim and 4 for the second. It then
 * requests resource 0x47 through the shared func_8014DD3C helper, stores 0x80 in
 * work-record byte 0x24 and 1 in byte 0x29, clears bytes 0x2A/0x48, stores the
 * halfwords 0x5E at 0x2E and 0xC2 at 0x30, clears bytes 9, 0xA and 0x5D..0x5F,
 * and finally increments dispatch byte 3 of the record published at the
 * scratchpad cursor 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 * The three cursor locals and the direct D_1F800044 expressions are deliberate:
 * the original reloads the volatile pointer cell per store group (the 0x48 and
 * 0x2E/0x30 stores share one load, the byte-9 store has its own), so each reload
 * point gets its own quantity instead of one reused cursor.
 */
void initializeTwoEntryRecordsAndAdvanceStateWorld00Area03004_801D57B0(void) {
  u32 index;
  u8* panel;
  u8* timer;
  u8* dispatch;

  index = (u8)func_8019601c();
  if (index != 0xFF) {
    D_80143FC8[index].flags_00 = 1;
    D_80143FC8[index].byte_05 = 26;
    D_80143FC8[index].unk_01 = 16;
    D_80143FC8[index].unk_03 = 2;
  }
  index = (u8)func_8019601c();
  if (index != 0xFF) {
    D_80143FC8[index].flags_00 = 1;
    D_80143FC8[index].byte_05 = 26;
    D_80143FC8[index].unk_01 = 16;
    D_80143FC8[index].unk_03 = 4;
  }
  func_8014DD3C(0x47);
  D_1F800044[0x24] = 0x80;
  D_1F800044[0x29] = 1;
  D_1F800044[0x2A] = 0;
  panel = D_1F800044;
  panel[0x48] = 0;
  timer = D_1F800044;
  *(u16*)(panel + 0x2E) = 0x5E;
  *(u16*)(panel + 0x30) = 0xC2;
  timer[9] = 0;
  D_1F800044[0xA] = 0;
  D_1F800044[0x5D] = 0;
  D_1F800044[0x5E] = 0;
  D_1F800044[0x5F] = 0;
  dispatch = D_1F800044;
  {
    u8 next = dispatch[3];
    dispatch[3] = next + 1;
  }
}
