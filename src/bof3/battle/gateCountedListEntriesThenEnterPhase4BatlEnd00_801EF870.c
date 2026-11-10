#include "bof3/bof3.h"

extern u8 D_801462E1;
extern u8 D_801462E2;
extern u8 D_801462E3;
extern u8 D_801462E4;
extern u8 D_80146327; /* @source 0x80146327 @kind data */
extern u8 D_80146354[]; /* @source 0x80146354 @kind data */

/* Shared record gate of the main executable: the plchar, world and ui lifts of
 * this repository pass it a record family byte, a record id and a count and read
 * its byte answer, so it is declared here for its address; this body discards
 * the answer it gets back. */
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3); /* @source 0x801650B4 */

/* @source 0x801EF870
 * @behavior Battle-end counted-list gate and phase change, run once the
 *   EXE-side EMI loader reports ready through the shared ready test at
 *   0x80162D00: walks the counted list -- length byte at 0x80146327, halfword
 *   ids at 0x80146334, per-id counts at 0x80146354 -- and passes each entry to
 *   the shared record gate at 0x801650B4 as the id's high byte, the id's low
 *   byte and that entry's count with a zero fourth argument, discarding the
 *   answer; then sets the battle mode byte at 0x801462E1 to 4 and clears the
 *   mode bytes at 0x801462E2, 0x801462E3 and 0x801462E4. The length byte is
 *   re-read on every loop test, so the walk stops as soon as the list shrinks
 *   below the running index, and the walk index is zeroed ahead of that test,
 *   so the zeroing sits in the guard branch's delay slot. Takes no arguments
 *   and returns nothing; the 0x20-byte frame keeps $ra and $s0-$s1 across the
 *   per-entry calls.
 * @status exact
 * @match 100.00
 * @residual none
 * 0x801EF870 is entry 13, the last one, of the substate table based at
 * 0x801F0C98 that dispatchSubstateTable0c98BatlEnd00_801EEC58 indexes with the
 * byte at 0x801462E3, so this body is the tail of that run. The mode byte it
 * stores is the one the shared battle dispatcher takes as its handler-table
 * index (D_801EACD4 read at 0x801D6774).
 */
void gateCountedListEntriesThenEnterPhase4BatlEnd00_801EF870(void) {
  u8* count;
  u16* ids;
  u16 id;
  u8 i;

  if (func_80162D00() != 0) {
    count = &D_80146327;
    i = 0;
    if (*count != 0) {
      ids = (u16*)(count + 0xd);
      do {
        id = ids[i];
        func_801650B4(id >> 8, id & 0xff, D_80146354[i], 0);
        i += 1;
      } while (i < *((u8*)ids - 0xd));
    }
    D_801462E1 = 4;
    D_801462E2 = 0;
    D_801462E3 = 0;
    D_801462E4 = 0;
  }
}
