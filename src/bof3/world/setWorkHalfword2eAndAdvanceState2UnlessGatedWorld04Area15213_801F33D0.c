#include "bof3/bof3.h"

/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* @source 0x801F33D0
 * @behavior Entry 0 of the work-byte-2 handler run based at 0x801F4D74 in this
 *           overlay: the word at that base is 0x801F33D0, so this is the state
 *           the dispatcher 0x801F338C reaches while work record byte 0x02 still
 *           reads 0. When the shared mode byte 0x801454F2 reads 2 it returns,
 *           and it also returns when bit 0x100 of the shared halfword
 *           0x80146258 is set. Otherwise it writes -0x30 to halfword 0x2E of the
 *           work record published at the scratchpad pointer cell 0x1F800044 and
 *           increments that record's state byte 0x02, the old byte value read
 *           before the halfword store. It takes no arguments, uses no result
 *           and reserves no frame. Its 0x50 bytes were byte-identical in one
 *           cmp to the exact
 *           setWorkHalfword2eAndAdvanceState2UnlessGatedWorld03Area15113_801F347C
 *           lift in emi/world03/area151/13 (span sha1
 *           4ca063c72b6974be9523ebc147761eec9e8abb55, the span sha1 this
 *           family's already-exact lifts record), which supplied this body's
 *           statement order, operand forms and name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfword2eAndAdvanceState2UnlessGatedWorld04Area15213_801F33D0(
    void) {
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
