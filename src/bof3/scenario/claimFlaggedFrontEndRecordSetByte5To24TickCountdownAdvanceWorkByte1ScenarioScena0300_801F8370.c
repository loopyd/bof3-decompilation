#include "bof3/bof3.h"

extern u8 *D_1F800044;
/* @source 0x80143FC8 @kind table */
extern u8 D_80143FC8[];

/* @source 0x801F8370
 * @behavior Entry 1 of the overlay's work-object handler table at 0x801FCF94
 * (the three words 0x801F8328 / 0x801F8370 / 0x801F842C), which the frame
 * dispatcher dispatchWorkByte1HandlerScenarioScena0300_801F82E4 selects with
 * unsigned byte 0x1 of the work object published at scratchpad pointer cell
 * 0x1F800044 and calls with no arguments: while that object's countdown byte
 * at +0x9 still has bit 2 set, it takes one record index from func_8019601C
 * and, unless the index reads 0xFF, sets row (index * 0x74) of the front-end
 * record table at 0x80143FC8 so that byte 0x00 reads 1 and byte 0x05 reads
 * 0x24. It then decrements the same countdown byte and, when the decremented
 * byte is zero, advances work byte 0x01 by one so the next frame dispatches
 * entry 2 (0x801F842C); after that decrement the object is no longer flagged
 * and no further record is claimed. The pointer cell is re-read for the
 * decrement group and again for the trailing increment, because the record
 * bytes stored in between may alias that cell. Takes no arguments, returns
 * nothing and reads no other state; the 0x18-byte frame keeps $ra across the
 * index call. The roles of record byte 0x05 beyond this state value and of the
 * bit-2 flag beyond holding the claim loop open are unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFlaggedFrontEndRecordSetByte5To24TickCountdownAdvanceWorkByte1ScenarioScena0300_801F8370(
    void) {
  u8 *record;
  u8 index;

  if (D_1F800044[9] & 4) {
    index = func_8019601C();
    if (index != 0xFFu) {
      record = &D_80143FC8[index * 0x74];
      record[0] = 1;
      record[5] = 0x24;
    }
  }
  if (--D_1F800044[9] == 0) {
    D_1F800044[1]++;
  }
}
