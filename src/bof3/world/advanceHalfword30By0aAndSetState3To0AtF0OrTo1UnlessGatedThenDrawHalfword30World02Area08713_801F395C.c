#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* scratchpadWorkRecord;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

void drawSpriteFrameAndMainRamTexture0World02Area08713_801F40F8(s16 arg0,
                                                                s16 arg1);

/* @source 0x801F395C
 * @behavior Entry 3 of the four-entry work-byte-3 handler run based at
 *           0x801F5760, whose fourth pointer word 0x801F576C names this
 *           address; the byte-3 dispatcher 0x801F36C0 indexes that run with
 *           record byte 0x03. Over the work record published at the scratchpad
 *           pointer cell 0x1F800044 it adds 0x0A to the halfword at record
 *           offset 0x30 and stores the truncated halfword, then clears work
 *           byte 0x03 once that stored signed halfword has reached 0xF0. It
 *           writes 1 to work byte 0x03 through a second read of the pointer
 *           cell unless the shared mode byte 0x801454F2 is nonzero, the byte at
 *           0x80143BB0 is 2, or bit 0x100 of halfword 0x80146258 is set.
 *           Finally it reads the cell once more and passes 0x5C and the signed
 *           halfword at 0x30 to the local emitter 0x801F40F8. It takes no
 *           arguments, uses no result and saves only the return address in a
 *           0x18-byte frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 0xB0 bytes at payload offset 0xD5C differ in one word from the span at
 * offset 0xD40 in emi/world03/area121/14, the already-lifted
 * advanceHalfword30By0aAndSetState3To0AtF0OrTo1UnlessGatedThenDrawHalfword30World03Area12114_801F3940:
 * that word is the `jal` at span+0x98, whose decoded target 0x801F40DC sits
 * 0x1C below this overlay's own emitter 0x801F40F8, the same quantity as the
 * 0x1C between the two spans, so all 43 other words are equal in one cmp of
 * each payload span at its own offset. A `jal`-masked payload scan of the 398
 * binaries under out/binaries found the same 0xB0 bytes in eight further
 * overlays and no byte-identical span, so that sibling supplied this body's
 * statement order and operand forms with this overlay's own emitter address.
 */
void advanceHalfword30By0aAndSetState3To0AtF0OrTo1UnlessGatedThenDrawHalfword30World02Area08713_801F395C(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = scratchpadWorkRecord;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x0A);
  if (*half >= 0xF0) {
    record[3] = 0;
  }
  if (D_801454F2 == 0 && D_80143BB0 != 2 && (D_80146258 & 0x100u) == 0) {
    scratchpadWorkRecord[3] = 1;
  }
  emitter = scratchpadWorkRecord;
  drawSpriteFrameAndMainRamTexture0World02Area08713_801F40F8(
      0x5C, FIELD_REF(s16, emitter, 0x30));
}
