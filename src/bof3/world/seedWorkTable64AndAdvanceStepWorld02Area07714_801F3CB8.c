#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x80146954 @kind unknown */
extern u32 workSeedWord80146954;
/* @source 0x80146958 @kind unknown */
extern u32 workSeedWord80146958;
/* @source 0x8014695C @kind unknown */
extern u32 workSeedWord8014695C;

void seedWorkTable64World02Area07714_801F41B4(void);

/* @source 0x801F3CB8
 * @behavior Copies the shared words at 0x80146954/0x80146958/0x8014695C
 *           into scratch work-record words +0x34/+0x38/+0x3C, sets work
 *           word +0x0C to 180, calls the local 64-record table seeder,
 *           then reloads the scratchpad work pointer at 0x1F800044 and
 *           increments its dispatch byte +1. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkTable64AndAdvanceStepWorld02Area07714_801F3CB8(void) {
  u8* work;
  u8* nextWork;

  work = workRecordCursor1F800044;
  *(u32*)(work + 0x34) = workSeedWord80146954;
  *(u32*)(work + 0x38) = workSeedWord80146958;
  *(u32*)(work + 0x3C) = workSeedWord8014695C;
  *(u32*)(work + 0x0C) = 180;
  seedWorkTable64World02Area07714_801F41B4();
  nextWork = workRecordCursor1F800044;
  nextWork[1]++;
}
