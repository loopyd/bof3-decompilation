#include "bof3/scenario/sce10eff_internal.h"

/* @source 0x801D0C70
 * @behavior Seeds nine slots of the 0x74-byte front-end record table at
 * 0x80143FC8 after caching the scratchpad work pointer at 0x1F800044 into
 * 0x801D2744: one record for slot 0 (bytes 0x00 = 1, 0x05 = 0x69, 0x01 = 2,
 * 0x02 = 0, 0x03 = 0), then four records for slot 1 and four more for slot 2,
 * claiming one record per iteration from func_8019601C and storing the 0..3
 * iteration counter in record byte 0x04 and the slot number 1 then 2 in record
 * byte 0x02 (bytes 0x00 = 1, 0x05 = 0x69, 0x01 = 2, 0x03 = 0 in both groups);
 * finally clears work byte 0x0B, increments work byte 0x01 and queues cue
 * 0x203 through func_8015DF18.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D0C70(void) {
  u8 index;
  u8 i;

  D_801D2744 = (u8*)D_1F800044;

  index = func_8019601C();
  D_80143FC8[index].flags_00 = 1;
  D_80143FC8[index].unk_05 = 0x69;
  D_80143FC8[index].unk_01 = 2;
  D_80143FC8[index].unk_02 = 0;
  D_80143FC8[index].unk_03 = 0;

  for (i = 0; i < 4; i++) {
    index = func_8019601C();
    D_80143FC8[index].unk_04 = i;
    D_80143FC8[index].flags_00 = 1;
    D_80143FC8[index].unk_05 = 0x69;
    D_80143FC8[index].unk_01 = 2;
    D_80143FC8[index].unk_02 = 1;
    D_80143FC8[index].unk_03 = 0;
  }

  for (i = 0; i < 4; i++) {
    index = func_8019601C();
    D_80143FC8[index].unk_04 = i;
    D_80143FC8[index].flags_00 = 1;
    D_80143FC8[index].unk_05 = 0x69;
    D_80143FC8[index].unk_01 = 2;
    D_80143FC8[index].unk_02 = 2;
    D_80143FC8[index].unk_03 = 0;
  }

  ((u8*)D_1F800044)[0x0b] = 0;
  ((u8*)D_1F800044)[1]++;
  func_8015DF18(0x203);
}
