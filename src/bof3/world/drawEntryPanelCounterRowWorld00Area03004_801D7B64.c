#include "bof3/world/area03004_internal.h"

/* @source 0x801D7B64
 * @behavior draws the AREA030 entry-panel counter row at its fixed coordinates: selects
 * draw mode 0/1 and stamps sprite 0x12 at (0xF0, 0x42), submits the
 * two-character rodata label D_801D0C10 at (0x11C, 0x45) through the shared
 * text drawer func_8014FF0C, then formats the signed halfword at offset 0x3E
 * of the work counterWork published at the scratchpad cursor 0x1F800044 plus
 * one through the "%2d" format into the shared 0x80145AD4 buffer, rewrites a
 * leading space of that buffer to 0xFF and submits the two-token run at (0xFA,
 * 0x45) through drawEncodedTextAt.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEntryPanelCounterRowWorld00Area03004(void) {
  u8* textBuffer;
  u8* counterWork;

  submitTpageDrawMode(0, 1);
  appendIndexedSpriteWorld00Area03004(0x12, 1, 0xF0, 0x42);
  func_8014FF0C(0x11C, 0x45, 0, D_801D0C10);

  textBuffer = D_80145AD4;
  counterWork = D_1F800044;
  sprintf((char*)textBuffer, (const char*)D_801D0C04, FIELD_REF(s16, counterWork, 0x3E) + 1);
  if (textBuffer[0] == 0x20) {
    textBuffer[0] = 0xFF;
  }
  drawEncodedTextAt(0xFA, 0x45, 0, 2, (u32)textBuffer);
}
