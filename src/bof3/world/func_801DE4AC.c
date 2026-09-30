#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DE4AC
 * @behavior Walks the 0x1E shared 0x98-byte work records at 0x80146888; for
 * each active record it publishes the record as the scratchpad cursor at
 * 0x1F800044 and as D_80146884, dispatches the record's handler byte 0x01
 * through the overlay handler table D_801E2314, then updates the record's
 * level byte 0x5D from the condition flag 0x20 and the signed counter at
 * 0x3E and mirrors that level into 0x5E and 0x5F.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 80/80 instructions, 320 bytes, live byte match.
 * The three pointer locals are load bearing: `record` publishes the walked
 * record, `work` re-reads the cursor for the level update, and `cur` is a
 * byte view of the record for the 0x5E/0x5F mirrors; collapsing any pair
 * changes the caller-side register web. The level field is signed so both
 * level stores share the hoisted -0x40 constant.
 */
void func_801DE4AC(void) {
  Area030WorkRecord* record;
  Area030WorkRecord* work;
  u8* cur;
  s32 i;
  s16 counter;

  for (i = 0; i < 0x1E; i++) {
    if (D_80146888[i].flags_00 != 0) {
      record = &D_80146888[i];
      D_1F800044 = (u8*)record;
      D_80146884 = (Area030Range*)record;
      D_801E2314[record->handler_01]();
      work = (Area030WorkRecord*)D_1F800044;
      if (work->flags_00 & 0x20) {
        counter = work->counter_3E;
        if (counter > 0) {
          work->level_5D = -0x40;
        } else if (counter < -0x200) {
          work->level_5D = 0;
        } else {
          work->level_5D = -0x40 - counter / 8;
        }
      } else {
        work->level_5D = 0;
      }
      cur = D_1F800044;
      cur[0x5E] = cur[0x5D];
      cur = D_1F800044;
      cur[0x5F] = cur[0x5D];
    }
  }
}
