#include "bof3/bof3.h"

/* Shared field record of the main executable this overlay edits: a 164-byte
 * record whose slot bytes sit at 0x0E through 0x13, the same six bytes the
 * overlay helper setFieldRecordSlotValue writes by slot. It is rebuilt here
 * through func_80164A44 once the slots below have been committed. */
extern u8 D_80144DE4[]; /* @source 0x80144DE4 @kind data */

/* First of the six shared slot bytes 0x80144DF2 through 0x80144DF7, which are
 * bytes 0x0E through 0x13 of the field record above. The six pointer cells this
 * body builds on its frame are the addresses of these bytes. */
extern u8 D_80144DF2[]; /* @source 0x80144DF2 @kind data */

/* Six-slot helper-group table of this overlay: the shipped bytes are 1, 2, 2,
 * 2, 3, 3, the same group per slot the overlay helper setFieldRecordSlotValue
 * passes to the two main-exe calls below. It sits inside the trailing data blob
 * of the payload, so the generated map already names it. */
extern u8 D_801D875C[]; /* @source 0x801D875C @kind data */

/* Six-byte pending-slot snapshot of this overlay, re-seeded from the shared
 * slot bytes by the sibling publishBateMaskFlagAndSnapshotSharedFields. A zero
 * byte and a byte equal to the shared slot byte both mean nothing to commit. */
extern u8 D_801D8EE0[]; /* @source 0x801D8EE0 @kind data */

/* Main-exe slot setter: the helper group of the slot, the new byte value, 1 and
 * 0. Its result is discarded here, so the widths are the ones observed at the
 * reviewed call sites. */
extern u8 func_801665A0(s32 arg0, s32 arg1, s32 arg2, s32 arg3); /* @source 0x801665A0 */

/* Main-exe gate called with the same helper group and the shared slot byte the
 * store has not replaced yet. Its result is discarded here. */
extern u8 func_801650B4(s32 arg0, s32 arg1, s32 arg2, s32 arg3); /* @source 0x801650B4 */

/* Main-exe field-record rebuild helper, called with the address of the record
 * whose slot bytes this body just updated. */
extern void func_80164A44(void *record); /* @source 0x80164A44 */

/* @source 0x801D1E8C
 * @behavior Commits the pending six-slot snapshot of the BATE overlay into the
 *           six shared slot bytes of the field record 0x80144DE4 and then
 *           rebuilds that record. It walks the six slots in order, each slot's
 *           shared byte taken through its own pointer cell: a slot is skipped
 *           when its snapshot byte is zero and skipped too when that snapshot
 *           byte already equals the shared slot byte, so an unchanged or empty
 *           slot costs no call. A differing slot first calls the main-exe slot
 *           setter func_801665A0 with the slot's helper group from the six-byte
 *           table 0x801D875C, the snapshot byte, 1 and 0; it then calls the
 *           main-exe gate func_801650B4 with the same group and the shared byte
 *           the store has not replaced yet, and only then stores the snapshot
 *           byte into the shared slot byte. Both helper answers are discarded.
 *           After the walk it rebuilds the field record 0x80144DE4 through
 *           func_80164A44. Nothing is returned; the 0x38-byte frame keeps the
 *           return address at 0x34, the three saved registers at 0x28 through
 *           0x30 and the six pointer cells 0x10 through 0x27.
 * @status exact
 * @match 100.00
 * @residual none
 */
void commitBateSlotSnapshotIntoSharedFieldRecord(void) {
  u8 *slotFields[6];
  u8 i;

  slotFields[0] = &D_80144DF2[0];
  slotFields[1] = &D_80144DF2[1];
  slotFields[2] = &D_80144DF2[2];
  slotFields[3] = &D_80144DF2[3];
  slotFields[4] = &D_80144DF2[4];
  slotFields[5] = &D_80144DF2[5];

  for (i = 0; i < 6; i++) {
    if (D_801D8EE0[i] == 0) {
      continue;
    }
    if (D_801D8EE0[i] == *slotFields[i]) {
      continue;
    }
    func_801665A0(D_801D875C[i], D_801D8EE0[i], 1, 0);
    func_801650B4(D_801D875C[i], *slotFields[i], 1, 0);
    *slotFields[i] = D_801D8EE0[i];
  }
  func_80164A44(D_80144DE4);
}
