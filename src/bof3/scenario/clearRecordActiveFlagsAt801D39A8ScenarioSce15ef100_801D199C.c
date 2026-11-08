#include "bof3/bof3.h"

/* Table of 0x10 records of 0x18 bytes at 0x801D39A8 inside this overlay. The
 * lead byte of each record is its active flag: func_801D19C8 sets it to 1 and
 * fills the rest of the record from the scratchpad work object at 0x1F800044,
 * and func_801D1A60, now lifted as
 * stepClaimedWorkRecordsDecrementLifePairClearExpiredAndDrawAt801D39A8ScenarioSce15ef100_801D1A60,
 * gates its per-record update on it and clears it when the record byte-2
 * countdown expires. The bytes sit inside the Splat `textbin`
 * row at payload offset 0x2D18 and are zero on disc, so the run is only
 * referenced here. */
extern u8 D_801D39A8[]; /* @source 0x801D39A8 @kind data */

/* @source 0x801D199C
 * @behavior Overlay init helper: clears the lead byte of each of the 0x10
 * entries of the 0x18-byte-stride record table at 0x801D39A8, stepping the
 * entry pointer by 0x18 and an unsigned byte counter by 1 in a bottom-tested
 * loop. It takes no arguments, returns nothing and reads no other state. The
 * only in-payload caller is func_801D0DD8 (jal at 0x801D0E80), which bumps the
 * scratchpad work object's byte 1 in the delay slot and then walks the
 * 0x2C-byte-stride table at 0x801D3B28.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordActiveFlagsAt801D39A8ScenarioSce15ef100_801D199C(void) {
  u8* record;
  u8 i;

  record = D_801D39A8;
  i = 0u;

  do {
    *record = 0;
    record += 0x18u;
    i += 1u;
  } while (i < 0x10u);
}
