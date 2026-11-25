#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* scratchpadWorkRecord;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
extern u8 D_80143BB0;
extern u16 D_80146258;

/* The local emitter at 0x801F4C28 narrows both arguments with `sll`/`sra 16`
 * in its own body and this call site consumes no result. */
void func_801F4C28(s16 arg0, s16 arg1);

/* @source 0x801F448C
 * @behavior Handler of the four-entry state-byte-3 handler run based at
 *           0x801F61CC, over the work record published at the scratchpad
 *           pointer cell 0x1F800044. It adds 0x0A to the halfword at record
 *           offset 0x30 and stores the truncated halfword, then clears work
 *           byte 0x03 once that stored value read back as a signed halfword has
 *           reached 0xF0. Unless the shared byte 0x801454F2 is nonzero, the byte
 *           at 0x80143BB0 is 2, or bit 0x100 of halfword 0x80146258 is set, it
 *           writes 1 to work byte 0x03 through a second read of the pointer
 *           cell. Finally it reads the cell once more and passes 0x5C and the
 *           signed halfword at offset 0x30 to the local emitter 0x801F4C28. It
 *           takes no arguments, uses no result and saves only the return address
 *           in a 0x18-byte frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * One payload cmp of the 0xB0 bytes at offset 0x188C against the span at
 * offset 0xD5C of emi/world02/area087/13, the already-lifted
 * advanceHalfword30By0aAndSetState3To0AtF0OrTo1UnlessGatedThenDrawHalfword30World02Area08713_801F395C,
 * reports exactly two differing bytes, the two low bytes of the `jal` word at
 * span+0x98 (0x0C07D30A here, that overlay's own emitter 0x801F40F8 there), so
 * all 43 other words are equal and the two decoded emitter addresses sit 0xB30
 * apart, the same quantity as between the two spans. A `jal`-masked scan of the
 * 398 payloads under out/binaries found ten carriers of this body and no
 * byte-identical span, so clone was declined and that sibling supplied this
 * body's statement order and operand forms with this overlay's own emitter.
 */
void stateHalfword30Reset2(
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
  func_801F4C28(0x5C, FIELD_REF(s16, emitter, 0x30));
}
