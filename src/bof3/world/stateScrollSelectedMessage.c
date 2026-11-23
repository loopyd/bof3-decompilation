#include "bof3/world/area03004_internal.h"

/* @behavior advances the panel scroll, then draws one selected encoded-text run selected
 * by the scratch byte at +0x4B: entry table 0x801E1E78 supplies the trailing
 * byte and word arguments, and the scratch halfword at +0x30 plus two supplies
 * the y argument.
 * @source 0x801D3898
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateScrollSelectedMessage(void) {
  u8* cursor;
  u8 messageIndex;

  advancePanelScroll();
  cursor = (u8*)D_1F800044;
  messageIndex = cursor[0x4Bu];
  drawEncodedTextAt(29, (s16)(*(volatile u16*)(cursor + 0x30u) + 2), 0,
                D_801E1E78[messageIndex].byte_00, D_801E1E78[messageIndex].word_04);
}
