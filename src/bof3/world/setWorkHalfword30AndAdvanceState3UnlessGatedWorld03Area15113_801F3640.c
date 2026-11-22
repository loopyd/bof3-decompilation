#include "bof3/bof3.h"

extern u8 D_801454F2;
extern u8 D_80143BB0;
extern u16 D_80146258;

/* @source 0x801F3640
 * @behavior Entry 0 of the four-entry work-byte-3 handler table based at
 *           0x801F5458 in this overlay, reached through the dispatcher at
 *           0x801F35FC. When the shared mode byte 0x801454F2 is set and byte
 *           0x0B of the work record published at scratchpad pointer cell
 *           0x1F800044 is nonzero it returns. It then returns when byte
 *           0x80143BB0 is 2 or bit 0x100 of halfword 0x80146258 is set.
 *           Otherwise it reads the record through a second lookup of the
 *           pointer cell, writes 0xF0 to work-record halfword 0x30 and
 *           increments work byte 0x03, the old byte being read before the
 *           halfword store. Takes no arguments, uses no result and touches no
 *           other state.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 0x80 bytes at payload offset 0xA40 are byte-identical to the already-exact
 * setWorkHalfword30AndAdvanceState3UnlessGatedWorld01Area04513_801F36D0 span at
 * payload offset 0xAD0 in emi/world01/area045/13 in one cmp of each payload span
 * at its own offset, and its own span sha1
 * aed3fc233be4f6aa8c566f5cf99a6477c46a18bf is the one that comment records for
 * the World00Area03313_801F3604 (offset 0xA04), World00Area01613_801F36D0
 * (offset 0xAD0), World02Area08813_801F4234 (offset 0x1634),
 * World03Area12114_801F36E8 (offset 0xAE8) and World03Area11513_801F36D4
 * (offset 0xAD4) spans. This body carries no payload-relative word - every
 * absolute address it names is a shared cell outside the overlay - so the
 * family statement order, operand forms and the module-qualified name transfer
 * whole.
 */
void setWorkHalfword30AndAdvanceState3UnlessGatedWorld03Area15113_801F3640(
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
