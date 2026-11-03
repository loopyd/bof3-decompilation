#include "bof3/world/area03004_internal.h"

/* @behavior advances the panel scroll, then submits one shaded panel request
 * selected by the scratch byte at +0x4B: entry table 0x801E1E78 supplies the
 * trailing byte and word arguments, and the scratch halfword at +0x30 plus two
 * supplies the vertical origin.
 * @source 0x801D3898
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3898(void) {
  u8* cursor;
  u8 index;

  advancePanelScroll();
  cursor = (u8*)D_1F800044;
  index = cursor[0x4Bu];
  func_8014f800(29, (s16)(*(volatile u16*)(cursor + 0x30u) + 2), 0,
                D_801E1E78[index].byte_00, D_801E1E78[index].word_04);
}
