#include "bof3/world/area03004_internal.h"

/* @behavior If the selected four-byte row's last byte is nonzero, obtain an entry index.
 * Unless it is -1, initialize that entry's flags and selected bytes. Set work
 * handler byte 1 to 6 even for -1. Otherwise copy the row's first byte to work
 * byte 0x4B and advance handler byte 1. Request the fixed panel rectangle
 * last.
 * @source 0x801D36F8
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializePickedEntryOrAdvanceWorkHandlerWorld00Area03004(void) {
  u8* rowWork;
  u8* dispatchWork;
  s8 entryIndex;
  u8 rowSelector;

  rowWork = (u8*)D_1F800044;
  if (D_801E1F20[rowWork[6u]][3u] != 0u) {
    entryIndex = (s8)func_8019601c();
    if (entryIndex != -1) {
      D_80143FC8[entryIndex].flags_00 = 1u;
      D_80143FC8[entryIndex].byte_05 = 15u;
      rowSelector = D_1F800044[6u];
      D_80143FC8[entryIndex].byte_4A = 0u;
      D_80143FC8[entryIndex].byte_0B = 0u;
      D_80143FC8[entryIndex].unk_01 = 8u;
      D_80143FC8[entryIndex].byte_06 = rowSelector;
    }
    dispatchWork = (u8*)D_1F800044;
    dispatchWork[1u] = 6u;
  } else {
    rowWork[0x4Bu] = D_801E1F20[rowWork[6u]][0u];
    dispatchWork = (u8*)D_1F800044;
    dispatchWork[1u] = (u8)(dispatchWork[1u] + 1u);
  }
  drawFilledPanelWithInsetBorderWorld00Area03004(20, 18, 280, 19, 0);
}
