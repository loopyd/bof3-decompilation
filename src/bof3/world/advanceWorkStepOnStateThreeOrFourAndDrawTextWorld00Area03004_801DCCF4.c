#include "bof3/world/area03004_internal.h"

/* @behavior When shared state is 3 or 4, clear bit 7 of the panel flags, call
 * func_801647C4(28, maskedFlags * 16 + 88, 0), and advance a fresh work byte
 * 4. Then select the unsigned text offset at 0x800100A2 or 0x800100A4 using a
 * fresh flag-bit-0 read and draw the byte stream at 0x80010000 plus that
 * offset.
 * @source 0x801DCCF4
 * @status exact
 * @match 100.00
 * @residual none
 * First live seed scored 46/49 (93.88%) with only the bit-0 arm polarity
 * reversed: the flag-set path must be the out-of-line arm so the clear path
 * falls through to the shared tail, matching the AREA030 sibling rule recorded
 * in the lift-loop observations. Inverting the source condition to
 * `(D_80144287 & 1u) == 0u` matched live at 49/49 instructions.
 */
void advanceWorkStepOnStateThreeOrFourAndDrawTextWorld00Area03004(void) {
  u8 maskedFlags;

  if (D_80144281 - 3u < 2u) {
    maskedFlags = D_80144287 & 0x7Fu;
    D_80144287 = maskedFlags;
    func_801647C4(0x1Cu, (u16)(((u32)maskedFlags << 4) + 0x58u), 0);
    D_1F800044[4]++;
  }
  if ((D_80144287 & 1u) == 0u) {
    drawEncodedTextAt(0x1Du, 0x14, 0, 0xFFu, 0x80010000u + (u32)D_800100A2);
  } else {
    drawEncodedTextAt(0x1Du, 0x14, 0, 0xFFu, 0x80010000u + (u32)D_800100A4);
  }
}
