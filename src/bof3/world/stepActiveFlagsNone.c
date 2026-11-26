#include "bof3/bof3.h"

/* @source 0x801F31C0 */
extern u32 stateStepWorkRecords(void);

/* @source 0x80196070 */
extern void func_80196070(void);

/* @source 0x801F2E94
 * @behavior Third handler of this overlay's state-byte-1 dispatch table at
 *           0x801F44B8: its pointer word is table entry 2 at 0x801F44C0. It
 *           runs the 0x800E4800 work-record step stateStepWorkRecords, which walks
 *           the 0x10 records of the 0x18-byte-stride block whose occupancy
 *           bytes start at 0x800E4815, draws each occupied record through
 *           func_801F2EC8 and answers 1 when any record was occupied, 0 when
 *           none was. Only while that answer reads 0 - no record of the
 *           shared work area is active - does it reach the shared work-flag
 *           reset func_80196070, so the overlay clears the shared flags on a
 *           step that finds nothing left to animate. It takes no arguments,
 *           returns nothing and keeps no state of its own; the 0x18-byte
 *           frame only holds $ra at 0x10 across the two calls. The callee
 *           prototype is the caller-side width the original consumes: only
 *           the low byte of its answer is tested (`andi $v0, $v0, 0xFF` at
 *           0x801F2EA4). Its 0x34 bytes have the same shape as the
 *           resetWorkFlagsOnZeroRefreshScenarioScena0800_801F7A9C lift, which
 *           had matched exact with the same low-byte test and the same second
 *           call; only the first call's target word differs, so this target
 *           owns its own address and boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepActiveFlagsNone(void) {
  if ((u8)stateStepWorkRecords() == 0) {
    func_80196070();
  }
}
