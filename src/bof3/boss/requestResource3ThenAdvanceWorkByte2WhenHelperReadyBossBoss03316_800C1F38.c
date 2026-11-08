#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so the load folds to lui plus an offset load, and because a store
 * through the loaded pointer may alias the cell the original re-reads it after
 * the resource request instead of keeping the pointer across the call. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Readiness poll helper of the main executable. The battle and bmagic lifts
 * declare the same prototype; the original masks the returned byte with 0xFF
 * before testing it, so the declared return type is the byte it reports. */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper; the sibling boss and bmagic
 * lifts declare the same prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag);

/* @source 0x800C1F38
 * @behavior Work-step handler at index 1 of this overlay's code-pointer run
 *           based at 0x800C27B4, the base word the step dispatcher
 *           func_800C1E70 indexes with work byte 2 scaled by four; run word 1 at
 *           0x800C27B8 holds this address, and no jal in this payload names it.
 *           It calls the readiness helper func_8014DAEC and keeps only that
 *           helper's returned byte as a ready test. While the helper reports not
 *           ready the handler writes nothing at all. Once it reports ready the
 *           handler requests resource 3 through func_8014D6B8, then loads the
 *           scratch work object published at the pointer cell
 *           0x1F800044 and advances that object's unsigned byte 2, the step
 *           index, by one, so the next dispatch selects the handler at index 2
 *           of the same run (0x800C1F88) instead of re-entering this one. Takes
 *           no arguments and returns nothing; reads or writes no other state,
 *           and the 0x18-byte frame only keeps the return address at 0x10
 *           across its two calls. The roles of resource 3 and of the work
 *           record are not established here, so the name describes the effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResource3ThenAdvanceWorkByte2WhenHelperReadyBossBoss03316_800C1F38(
    void) {
  if (func_8014DAEC()) {
    func_8014D6B8(3);
    D_1F800044[2]++;
  }
}
