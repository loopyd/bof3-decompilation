#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area15213WorkCursor_1F800044;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* @source 0x801F3594
 * @behavior Byte-3 state handler 0 of this overlay's handler run based at
 *           0x801F4D84, whose four words are 0x801F3594, 0x801F3614,
 *           0x801F36EC and 0x801F37EC. It returns when the shared byte
 *           0x801454F2 is nonzero and work byte 0x0B is nonzero, when the
 *           byte at 0x80143BB0 reads 2, or when bit 0x100 of the halfword at
 *           0x80146258 is set. Otherwise it writes 0xF0 to the signed halfword
 *           at offset 0x30 of the work record published at the scratchpad
 *           pointer cell 0x1F800044 and increments that record's byte 0x03,
 *           both through one read of the cell. It takes no arguments, returns
 *           nothing and saves only the return address at 0x10 of its
 *           0x18-byte frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfword30AndAdvanceState3UnlessGatedWorld04Area15213_801F3594(
    void) {
  u8* record;

  if (D_801454F2 != 0 && area15213WorkCursor_1F800044[0x0B] != 0) {
    return;
  }
  if (D_80143BB0 == 2) {
    return;
  }
  if ((D_80146258 & 0x100u) != 0) {
    return;
  }
  record = area15213WorkCursor_1F800044;
  FIELD_REF(s16, record, 0x30) = 0xF0;
  record[3]++;
}
