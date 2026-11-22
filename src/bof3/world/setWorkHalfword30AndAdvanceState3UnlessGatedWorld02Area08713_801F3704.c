#include "bof3/bof3.h"

extern u8 D_80143BB0;
extern u8 D_801454F2;
extern u16 D_80146258;

/* @source 0x801F3704
 * @behavior Entry 0 of the four-entry work-byte-3 handler run based at
 *           0x801F5760, whose first pointer word names this address. It returns
 *           when the shared mode byte 0x801454F2 is set and byte 0x0B of the
 *           work record published at the scratchpad pointer cell 0x1F800044 is
 *           nonzero, then returns when byte 0x80143BB0 is 2 or bit 0x100 of
 *           halfword 0x80146258 is set. Otherwise it writes 0xF0 to work-record
 *           halfword 0x30 through a second read of the pointer cell and
 *           increments work byte 0x03, which is the index the byte-3 dispatcher
 *           of this overlay reads. It takes no arguments, uses no result and
 *           saves nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 0x80 bytes at payload offset 0xB04 are byte-identical (span sha1
 * aed3fc233be4f6aa8c566f5cf99a6477c46a18bf) to the already-lifted
 * setWorkHalfword30AndAdvanceState3UnlessGatedWorld00Area03313_801F3604 span at
 * offset 0xA04 in emi/world00/area033/13, the
 * setWorkHalfword30AndAdvanceState3UnlessGatedWorld01Area04513_801F36D0 span at
 * offset 0xAD0 in emi/world01/area045/13, the
 * setWorkHalfword30AndAdvanceState3UnlessGatedWorld02Area08813_801F4234 span at
 * offset 0x1634 in emi/world02/area088/13 and the
 * setWorkHalfword30AndAdvanceState3UnlessGatedWorld03Area12114_801F36E8 span at
 * offset 0xAE8 in emi/world03/area121/14, in one cmp of each payload span at
 * its own offset. Those lifts supplied this body's statement order, operand
 * forms and the module-qualified name; the same span sits unchanged in the
 * emi/world00/area016/13 (0xAD0), emi/world01/area065/13 (0xB08),
 * emi/world03/area115/13 (0xAD4), emi/world03/area151/13 (0xA40) and
 * emi/world04/area152/13 (0x994) payloads, whose asm rows no source had claimed
 * when this seed was written.
 */
void setWorkHalfword30AndAdvanceState3UnlessGatedWorld02Area08713_801F3704(
    void) {
  u8* record;

  if (D_801454F2 != 0) {
    if (SPAD_PTR_SLOT(u8, 0x44)[0x0B] != 0) {
      return;
    }
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
