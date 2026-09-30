#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D3390
 * @behavior when the D_801D4284 byte is set, starts the selected master's +4
 *           action through func_80150224, writes the phase byte D_80143BB0 as 2,
 *           sets modeIndex to 6 and clears the handler index D_801D4286;
 *           otherwise copies the first five bytes of the record selected by the
 *           D_801D428A step through D_80145FCC into the D_801490D8 work buffer
 *           and clears its terminator, sets that record's owner byte D_80144983
 *           to 0xFF and clears the seven bytes at record offsets 0x84..0x8A,
 *           then starts the selected master's +0xC action through func_80150224,
 *           writes the phase byte as 2 and advances the handler index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3390(void) {
  s32 index;
  s32 offset;
  s32 record_offset;

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
  D_80144983[record_offset] = 0xFF;
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  D_80144968[record_offset + 0x84] = 0;
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  D_80144968[record_offset + 0x85] = 0;
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  D_80144968[record_offset + 0x86] = 0;
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  D_80144968[record_offset + 0x87] = 0;
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  D_80144968[record_offset + 0x88] = 0;
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  D_80144968[record_offset + 0x89] = 0;
  offset = D_801D428A * 0x140;
  record_offset = D_80145FCC[offset] * 164;
  D_80144968[record_offset + 0x8A] = 0;
  func_80150224((s16)(masterActionBaseTable[masterIndex] + 0xC));
  D_80143BB0 = 2;
  D_801D4286 = D_801D4286 + 1;
}
