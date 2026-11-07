#include "bof3/bof3.h"

/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* @source 0x801F3604
 * @behavior Entry 0 of the four-entry work-byte-3 handler run based at
 *           0x801F4F90 in this overlay, reached through the dispatch on work
 *           record byte 3. When the shared mode byte 0x801454F2 is set and byte
 *           0x0B of the work record published at scratchpad pointer cell
 *           0x1F800044 is nonzero it returns. It then returns when byte
 *           0x80143BB0 is 2 or bit 0x100 of halfword 0x80146258 is set.
 *           Otherwise it writes 0xF0 to work-record halfword 0x30 through a
 *           second read of the pointer cell and increments work byte 0x03, the
 *           old byte being read before the halfword store. It takes no
 *           arguments, uses no result and touches no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfword30AndAdvanceState3UnlessGatedWorld00Area03313_801F3604(
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
