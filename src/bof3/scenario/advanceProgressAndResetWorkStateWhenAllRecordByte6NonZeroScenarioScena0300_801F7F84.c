#include "bof3/bof3.h"

extern u8 *D_1F800044;
extern u8 g_ScenarioProgress;
void func_80196070(void);

/* @source 0x801F7F84
 * @behavior Entry 2 of the overlay's three-word work-object handler table
 * workByte1HandlersScenarioScena0300_801FCF88 (0x801F7E34 / 0x801F7E6C / this
 * function), which the frame dispatcher func_801F7DF0 selects with byte 0x1 of
 * the work object published at scratchpad pointer slot 0x1F800044. It walks the
 * five record-pointer slots of that work object at offset 0x0C and returns as
 * soon as a slot's record reads zero at byte 0x06: entry 1 (0x801F7E6C) claims
 * exactly five records into those slots and stores zero in that byte, so this
 * handler waits while any claimed record still reads zero there. Once all five
 * records read a non-zero byte 0x06 it increments the shared scenario progress
 * byte g_ScenarioProgress (0x80146864) and resets the work area through the
 * shared func_80196070 helper, which clears work-object bytes 0x00-0x04
 * including the dispatch byte 1. Takes no arguments and returns nothing and
 * reads no other state. The 0x18-byte frame only keeps $ra across the call; the
 * five slots are indexed with the same byte-masked index entry 1 uses, and the
 * progress increment reads, adds and stores through one pointer local so the
 * single 0x8014 page base survives into the store, which lands in the call
 * delay slot. The role of the record byte 0x06 beyond this test is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceProgressAndResetWorkStateWhenAllRecordByte6NonZeroScenarioScena0300_801F7F84(
    void) {
  u8 *work;
  u8 **records;
  u8 *progress;
  u8 i;

  work = D_1F800044;
  records = (u8 **)(work + 0x0C);
  for (i = 0; i < 5; i++) {
    if (records[i][6] == 0) {
      return;
    }
  }
  progress = &g_ScenarioProgress;
  *progress = *progress + 1;
  func_80196070();
}
