#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 work-record phase check: compares the halfword at offset
 * 0x58 of the scratch work record published at the scratchpad cursor
 * 0x1F800044 with the per-index halfword of D_801E2354 selected by the record
 * byte at offset 0x06; when the two differ it calls the shared helper
 * func_8014D978, otherwise it increments the record phase byte at offset 0x02.
 * @source 0x801E01DC
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkModeAtIndexedOffsetElseStepSequenceWorld00Area03004(void) {
  u8* work;
  u8 offsetIndex;
  u16 sequenceOffset;

  work = D_1F800044;
  offsetIndex = work[6];
  sequenceOffset = *(u16*)(work + 0x58);
  if (sequenceOffset == D_801E2354[offsetIndex]) {
    work[2] = (u8)(work[2] + 1u);
  } else {
    func_8014D978();
  }
}
