#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 *D_80148208;
extern u8 D_80146874;
extern u8 D_80146875;

/* @source 0x801F4350
 * @behavior Reads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8). When the gate byte is zero it stores
 *           0xFFFF into the shared status halfword D_801490A8 (`ori v0,0,0xFFFF`,
 *           `sh`), publishes the work record pointer cell D_80148208 into a
 *           register and increments that record's unsigned halfword at offset
 *           0x7E in place (`lhu`, `addiu` 1, `sh`), then requests primary state
 *           0xD and sub-state 8 by storing 0xD into D_80146874 and 8 into
 *           D_80146875. When the gate byte is nonzero it stores only 0x35 into
 *           D_801490A8. The nonzero arm is the branch target, so its constant is
 *           materialised in the `bnez` delay slot and leaves through the single
 *           `jr $ra`; D_80146874 and D_80146875 are the shared primary-state and
 *           sub-state bytes the scenario overlays request states through. Takes
 *           no arguments, calls nothing, returns nothing; leaf with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus35IfGateElseResetStatusAndIncrementWorkHalfword7EState0DSubstate8World03Area12013_801F4350(
    void) {
  u8 *record;

  if (D_801490C7 == 0) {
    record = D_80148208;
    D_801490A8 = 0xFFFF;
    *(u16 *)(record + 0x7E) += 1;
    D_80146874 = 0xD;
    D_80146875 = 0x8;
  } else {
    D_801490A8 = 0x35;
  }
}
