#include "bof3/bof3.h"

extern u8 D_801454F2;
extern u16 D_80146258;

/* @source 0x801F3544
 * @behavior When the shared mode byte 0x801454F2 is not 2 and bit 0x100 of the
 *           world-flag halfword 0x80146258 is clear, writes -0x30 to the work
 *           record halfword at 0x2E and increments the record's state byte 0x02.
 *           The record is read from the scratchpad pointer cell 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfword2eAndAdvanceState2UnlessGatedWorld01Area06513_801F3544(void) {
  u8* record;

  if (D_801454F2 == 2) {
    return;
  }
  if ((D_80146258 & 0x100u) != 0) {
    return;
  }
  record = SPAD_PTR_SLOT(u8, 0x44);
  FIELD_REF(s16, record, 0x2E) = -0x30;
  record[2]++;
}
