#include "bof3/bof3.h"

/* Current AREA108 work-record cursor. The overlay seeds this cell with the
 * 0x800E4800 work-table base and walks it by the 0x28-byte record stride
 * (this function and func_801F3034 do that for the 16-record table), so the
 * cell always points at the record the per-frame handlers update. It is the
 * payload's last word and is zero on disc; the address is bound by this
 * target's generated undefined_syms_auto.txt, so no map row is needed. */
extern u8* workRecordCursor63ecWorld02Area10814; /* @source 0x801F63EC @kind data */

/* Per-record initialiser at 0x801F2F1C: copies the sketch record's position
 * words +0x34/+0x38/+0x3C from the scratch work record published at
 * 0x1F800044 into +0x04/+0x08/+0x0C, draws the three delta words at
 * +0x14/+0x18/+0x1C from rand, and sets the active byte +0x00 to 1, the
 * handler byte +0x01 to 0, the +0x02 countdown to 4, the +0x03 countdown to
 * 0x40 and the +0x24 halfword to 0. Calling it is what claims a record. */
void func_801F2F1C(u8* record);

/* @source 0x801F2FC8
 * @behavior Overlay init helper: publishes the main-RAM work-table base
 *           0x800E4800 into the record cursor cell at 0x801F63EC and then
 *           seeds the table by calling the per-record initialiser at
 *           0x801F2F1C for each of its 16 records of 0x28-byte stride,
 *           advancing the published cursor by one stride after every call. The
 *           cursor cell is read again for each access because the callee may
 *           write it, so the call argument and the advance are two separate
 *           loads of the cell. The walk is a bottom-tested loop with an
 *           unsigned byte counter compared below 0x10. Takes no arguments,
 *           returns nothing and keeps no state of its own; its 0x18-byte frame
 *           holds the saved $ra and the counter in $s0 across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedSixteenWorkRecordsWorld02Area10814_801F2FC8(void) {
  u8 count;

  workRecordCursor63ecWorld02Area10814 = PSX_PTR(u8, 0x800E4800u);
  count = 0u;
  do {
    func_801F2F1C(workRecordCursor63ecWorld02Area10814);
    workRecordCursor63ecWorld02Area10814 += 0x28u;
    count++;
  } while (count < 0x10u);
}
