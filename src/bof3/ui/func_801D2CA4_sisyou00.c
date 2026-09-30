#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D2CA4
 * @behavior when the D_801D4284 byte is set, starts the selected master's +4
 *           action through func_80150224, writes the phase byte D_80143BB0 as 2,
 *           sets modeIndex to 6 and clears the handler index D_801D4286;
 *           otherwise copies the first five bytes of the record selected by the
 *           D_801D428A step through D_80145FCC into the D_801490D8 work buffer
 *           and clears its terminator, sets that record's owner byte D_80144983
 *           to masterIndex, copies that record's own byte +6 into record offset
 *           0x84, copies the six per-master bytes D_801D4154[masterIndex*6 .. +5]
 *           into record offsets 0x85..0x8A, then starts the selected master's
 *           +0xA action through func_80150224, writes the phase byte as 2 and
 *           advances the handler index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D2CA4(void) {
  s32 index;
  s32 offset;
  s32 record_offset;
  u8* master;
  s32 cell;

  if (D_801D4284 != 0) {
    func_80150224((s16)(masterActionBaseTable[masterIndex] + 4));
    D_80143BB0 = 2;
    modeIndex = 6;
    D_801D4286 = 0;
    return;
  }
  for (index = 0; index < 5; index++) {
    u8* source;

    offset = D_801D428A * 0x140;
    source = D_80144968 + (D_80145FCC[offset] * 164);
    D_801490D8[index] = source[index];
  }
  D_801490D8[5] = 0;
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  master = &masterIndex;
  D_80144983[record_offset] = *master;
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  D_80144968[record_offset + 0x84] = D_80144968[record_offset + 6];
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  cell = *master * 6;
  D_80144968[record_offset + 0x85] = D_801D4154[cell];
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  cell = *master * 6;
  D_80144968[record_offset + 0x86] = D_801D4154[cell + 1];
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  cell = *master * 6;
  D_80144968[record_offset + 0x87] = D_801D4154[cell + 2];
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  cell = *master * 6;
  D_80144968[record_offset + 0x88] = D_801D4154[cell + 3];
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  cell = *master * 6;
  D_80144968[record_offset + 0x89] = D_801D4154[cell + 4];
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  cell = *master * 6;
  D_80144968[record_offset + 0x8A] = D_801D4154[cell + 5];
  func_80150224((s16)(masterActionBaseTable[*master] + 0xA));
  D_80143BB0 = 2;
  D_801D4286 = D_801D4286 + 1;
}
