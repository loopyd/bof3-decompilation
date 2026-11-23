#include "bof3/world/area03004_internal.h"

/* @source 0x801DEBC4
 * @behavior Handler 2 of D_801E2314. When func_8014D978 returns a nonzero byte, clears
 * D_801E3200, loads the selected scale record's entry_10, sets work flag 0x20,
 * restores the route row's trailing byte to work +0x2A, stores 2 at work +0x48
 * and selects the active-work sequence identified by that row's leading byte.
 * It then seeds work words +0x14/+0x0C/+0x10 with -2/0/0, stores 0x10 at work
 * +0x09 and selects handler 1. A zero helper result skips these stores and
 * further calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void restoreSlotEntryRestart(void) {
  u8* work;
  u8* scale_work;
  u8* route_work;

  if (func_8014D978()) {
    scale_work = D_1F800044;
    D_801E3200 = 0;
    applyMainLookupRecord(D_801E2AB8[scale_work[6]].entry_10);

    work = D_1F800044;
    work[0] = work[0] | 0x20;

    work = D_1F800044;
    work[0x2A] = D_801E2330[work[8]].byte_01;

    work = D_1F800044;
    work[0x48] = 2;

    route_work = D_1F800044;
    selectActiveWorkSequence(D_801E2330[route_work[8]].request_00);

    work = D_1F800044;
    FIELD_REF(s32, work, 0x14) = -2;
    FIELD_REF(s32, work, 0x0C) = 0;
    FIELD_REF(s32, work, 0x10) = 0;
    work[9] = 0x10;

    work = D_1F800044;
    work[1] = 1;
  }
}
