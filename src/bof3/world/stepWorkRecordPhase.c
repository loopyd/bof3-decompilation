#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads the cell afresh before every group of accesses in this body,
 * because the record step it calls may republish the object, so the source keeps
 * the named-cell form instead of caching the pointer in a local.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* @source 0x801F3084 */
extern void claimFourRecordsAround(void);

/* @source 0x801F31C0 */
extern void func_801F31C0(void);

/* @source 0x801F2DCC
 * @behavior Phase step of the 0x800E4800 work-record machinery, driven through
 *           the scratchpad work object 0x1F800044. It first tests the phase
 *           step byte 0x09 of that object: while it reads zero the four records
 *           of the current phase are still unclaimed, so it calls
 *           claimFourRecordsAround
 *           once to claim them. It then increments the step byte, and when the
 *           incremented value reaches 4 it wraps the step byte back to zero and
 *           increments the phase counter byte 0x06, so one phase is counted per
 *           four steps. The record step func_801F31C0 then advances every
 *           occupied record of the 0x800E4800 table. Finally, when the phase
 *           counter equals the limit the same object carries in byte 0x0B, it
 *           increments the work step byte 0x01. It takes no arguments, answers
 *           no value and returns nothing. The pointer cell is re-read before
 *           each group because the calls may republish it; the first pointer
 *           load precedes frame creation and the 0x18-byte frame only keeps $ra
 *           at 0x10 across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkRecordPhase(
    void) {
  if (D_1F800044[9] == 0) {
    claimFourRecordsAround();
  }
  D_1F800044[9] = D_1F800044[9] + 1;
  if (D_1F800044[9] == 4) {
    D_1F800044[9] = 0;
    D_1F800044[6] = D_1F800044[6] + 1;
  }
  func_801F31C0();
  if (D_1F800044[6] == D_1F800044[0xB]) {
    D_1F800044[1] = D_1F800044[1] + 1;
  }
}
