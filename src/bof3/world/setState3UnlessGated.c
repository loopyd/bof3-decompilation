#include "bof3/bof3.h"

extern u8 D_801454F2;
extern u8 D_80143BB0;
extern u16 D_80146258;

/* @source 0x801F36D4
 * @behavior Unless both the shared mode byte 0x801454F2 and work-record byte
 *           0x0B are nonzero, the phase byte 0x80143BB0 reads 2, or bit 0x100
 *           of the world-flag halfword 0x80146258 is set, writes 0xF0 to the
 *           work-record halfword at 0x30 and increments state byte 0x03. The
 *           record is read from the scratchpad pointer cell 0x1F800044 again
 *           after the gate tests. Takes no arguments and returns nothing.
 *           Reached as the entry the state-byte-3 handler table at 0x801F4F74
 *           holds at index 0, and as handler 4 of the state-byte-2 handler
 *           table based at 0x801F4F64.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setState3UnlessGated(void) {
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
