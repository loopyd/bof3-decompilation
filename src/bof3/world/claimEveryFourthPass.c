#include "bof3/bof3.h"

u8* findFreeWorkRecord(void);
void stateRecordDefaultsPosition(u8* record);
u32 tickDrawActiveRecords(void);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer for the guard read, again after the record
 * pass and once more inside the countdown-expiry arm, so the fixed-address cell
 * symbol keeps all three loads. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F2E88
 * @behavior Entry 1 of the byte-2 handler pointer run based at 0x801F3DEC, the
 *           run the dispatcher at 0x801F2DF4 indexes with the work record's
 *           byte at +0x02. On the frames whose countdown byte at +0x09 holds
 *           zero in its low two bits it asks this overlay's own free-entry scan
 *           at 0x801F3800 for the first unclaimed entry of the 0x20-entry,
 *           0x18-byte-stride record table at 0x800E4800 and, only when that
 *           scan returns one, seeds it with the overlay defaults and the work
 *           position triple at 0x801F37A4. Every frame it then runs the
 *           record update and draw pass at 0x801F38B0 and discards that pass's
 *           returned byte, reads the published work object again and counts
 *           its byte at +0x09 down; when the counted byte reaches zero it
 *           reads the work object once more and increments the work byte at
 *           +0x02, so the next step of the same dispatcher calls entry 2 at
 *           0x801F2F20. It takes no call argument, returns nothing and keeps
 *           only $ra across its three calls in an 0x18-byte frame.
 *
 * Name evidence: the only in-image word naming 0x801F2E88 is the single
 * pointer word at 0x801F3DF0, index 1 of the byte-2 handler run based at
 * 0x801F3DEC; no jal in the payload reaches this address, so the run's entries
 * are the whole of its call evidence. The neighbouring entries fix the roles
 * the name uses: entry 0 at 0x801F2E38, this overlay's earlier step, clears
 * the record table's claim bytes, stores 0x40 in the work object's byte at
 * +0x09 and increments the byte at +0x02 to reach this entry, so the +0x09
 * test and countdown here are that armed window and the +0x02 increment at
 * their expiry is the hand-off to entry 2 at 0x801F2F20, which does nothing
 * but re-run the record pass and call func_80196070 when that pass reports
 * nothing left to draw. The record identities behind the two calls come from
 * this target's own exact lifts: 0x801F3800 scans the 0x18-byte-stride table
 * at 0x800E4800 for a zero lead byte and 0x801F37A4 writes the default set and
 * the 0x34/0x38/0x3C position triple into the record it is handed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimEveryFourthPass(
    void) {
  u8* work;
  u8* record;
  u8* obj;
  u8* state;

  work = D_1F800044;
  if ((work[9] & 3) == 0) {
    record = findFreeWorkRecord();
    if (record != 0) {
      stateRecordDefaultsPosition(record);
    }
  }
  tickDrawActiveRecords();
  obj = D_1F800044;
  if (--obj[9] == 0) {
    state = D_1F800044;
    state[2] = state[2] + 1;
  }
}
