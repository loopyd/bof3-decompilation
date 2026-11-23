#include "bof3/bof3.h"

extern u8 D_80143BB0;
extern u8 D_801454F2;
extern u16 D_80146258;

/* @source 0x801F36D0
 * @behavior When the shared mode byte 0x801454F2 is set and byte 0x0B of the
 *           work record published at the scratchpad pointer cell 0x1F800044 is
 *           nonzero it returns. It then returns when byte 0x80143BB0 is 2 or
 *           bit 0x100 of halfword 0x80146258 is set. Otherwise it writes 0xF0
 *           to work-record halfword 0x30 through a second read of the pointer
 *           cell and increments work byte 0x03, the old byte being read before
 *           the halfword store; that byte is the index this overlay's
 *           work-byte dispatcher 0x801F368C reads. It takes no arguments, uses
 *           no result and touches no other state.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 0x80 bytes at payload offset 0xAD0 are byte-identical to the already-exact
 * setWorkHalfword30AndAdvanceState3UnlessGatedWorld00Area03313_801F3604 span at
 * payload offset 0xA04 in emi/world00/area033/13 in one cmp of each payload span
 * at its own offset. The same span sha1 aed3fc233be4f6aa8c566f5cf99a6477c46a18bf
 * is what the World00Area01613_801F36D0 (offset 0xAD0), World02Area08813_801F4234
 * (offset 0x1634) and World03Area12114_801F36E8 (offset 0xAE8) payload spans
 * hold, each proven by its own cmp, so one body supplies this body's statement
 * order, operand forms and the module-qualified name.
 */
void stateHalfword30Advance(
    void) {
  u8* record;

  if (D_801454F2 != 0 && SPAD_PTR_SLOT(u8, 0x44)[0x0B] != 0) {
    return;
  }
  if (D_80143BB0 == 2) {
    return;
  }
  if ((D_80146258 & 0x100u) != 0) {
    return;
  }
  record = SPAD_PTR_SLOT(u8, 0x44);
  FIELD_REF(u16, record, 0x30) = 0xF0;
  record[3]++;
}
