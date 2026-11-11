#include "bof3/bof3.h"

extern u8 D_801454F2;
extern u16 D_80146258;

/* @source 0x801F3540
 * @behavior When the shared mode byte 0x801454F2 is not 2 and bit 0x100 of the
 *           world-flag halfword 0x80146258 is clear, writes -0x30 to the work
 *           record halfword at 0x2E and increments the record's state byte 0x02.
 *           The record is read from the scratchpad pointer cell 0x1F800044.
 *           Its 0x50 bytes were byte-identical to the exact
 *           setWorkHalfword2eAndAdvanceState2UnlessGatedWorld01Area06513_801F3544
 *           lift in emi/world01/area065/13 and to
 *           setWorkHalfword2eAndAdvanceState2UnlessGatedWorld03Area11513_801F3510
 *           in emi/world03/area115/13 in one cmp of each payload span at its own
 *           offset (span sha1 4ca063c72b6974be9523ebc147761eec9e8abb55), which
 *           supplied this body's statement order and name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfword2eAndAdvanceState2UnlessGatedWorld02Area08713_801F3540(void) {
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
