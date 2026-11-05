#include "bof3/bof3.h"

/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* @source 0x801F36E8
 * @behavior Unless both byte 0x801454F2 and work byte 0x0B are nonzero,
 * byte 0x80143BB0 is 2, or bit 0x100 of halfword 0x80146258 is set,
 * writes 0xF0 to work halfword 0x30 and increments work state byte 0x03.
 * The work record is published in scratchpad pointer slot 0x44.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfword30AndAdvanceState3UnlessGatedWorld03Area12114_801F36E8(void) {
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
  FIELD_REF(s16, record, 0x30) = 0xF0;
  record[3]++;
}
