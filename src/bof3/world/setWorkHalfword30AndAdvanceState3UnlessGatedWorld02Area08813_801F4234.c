#include "bof3/bof3.h"

extern u8 D_80143BB0;
extern u8 D_801454F2;
extern u16 D_80146258;

/* @source 0x801F4234
 * @behavior Handler 0 of the four-entry state-byte-3 handler run based at
 *           0x801F61CC. When the shared mode byte 0x801454F2 is set and byte
 *           0x0B of the work record published at the scratchpad pointer cell
 *           0x1F800044 is nonzero it returns. It then returns when byte
 *           0x80143BB0 is 2 or bit 0x100 of halfword 0x80146258 is set.
 *           Otherwise it writes 0xF0 to work-record halfword 0x30 through a
 *           second read of the pointer cell and increments work byte 0x03.
 *           The original reloads the pointer cell in both groups; the stores
 *           share the second read as one base. It takes no arguments, uses no
 *           result and touches no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfword30AndAdvanceState3UnlessGatedWorld02Area08813_801F4234(
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
