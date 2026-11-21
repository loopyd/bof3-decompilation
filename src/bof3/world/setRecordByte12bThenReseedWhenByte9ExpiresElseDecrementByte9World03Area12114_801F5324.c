#include "bof3/bof3.h"

/* Scratchpad pointer cell 0x1F800044 holding the active work record. The cell is
 * a variable rather than a constant address so that the read of byte 0x09 and
 * the countdown store each reload it, as the original does.
 * @source 0x1F800044 @kind unknown
 */
extern u8* area12114WorkCursor_1F800044;

/* Record pointer cell of the shared main executable whose byte 0x12B this state
 * handler raises and the reseed probe at 0x801F5380 lowers. Its role beyond that
 * byte is not established.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

void probeMapCodeReseedSelectionElseBumpCountdownThenTurnOrClearWorkStateWorld03Area12114_801F5380(
    void);
void func_801B0DFC(void);

/* @source 0x801F5324
 * @behavior Byte-1 state handler 3 of the handler table 0x801F6894 and byte-2
 * state handler 1 of the overlapping table 0x801F689C, so the overlay step
 * dispatches at 0x801F45E0 and 0x801F4B00 both reach it with no arguments. It
 * raises byte 0x12B of the record published at 0x80146250 and then reads work
 * byte 0x09 of the scratchpad pointer cell 0x1F800044: a zero byte runs the
 * map-code reseed probe at 0x801F5380, while any other value stores that byte
 * minus one and runs the shared update at 0x801B0DFC. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRecordByte12bThenReseedWhenByte9ExpiresElseDecrementByte9World03Area12114_801F5324(
    void) {
  D_80146250[0x12B] = 1;
  if (area12114WorkCursor_1F800044[9] == 0) {
    probeMapCodeReseedSelectionElseBumpCountdownThenTurnOrClearWorkStateWorld03Area12114_801F5380();
  } else {
    area12114WorkCursor_1F800044[9] -= 1;
    func_801B0DFC();
  }
}
