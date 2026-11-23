#include "bof3/world/area03004_internal.h"

/* @behavior Update the record cursors, set D_801E3208 to 0xFF, and clear work words 0x0C
 * through 0x20 and byte 9. Select state 6 if either shared gate byte is zero,
 * otherwise state 0. Call func_8014D6DC(10, 2) or selectActiveWorkSequence(11)
 * according to the pointed byte at 0x79, then advance a fresh work byte 3.
 * @source 0x801D9D38
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWorkStepsSelect(void) {
  u8* dispatchWork;
  u8* resetWork;

  stateCursorsNonzeroIndices();
  resetWork = D_1F800044;
  D_801E3208 = 0xFF;
  *(u32*)(resetWork + 0x0C) = 0;
  *(u32*)(resetWork + 0x10) = 0;
  *(u32*)(resetWork + 0x14) = 0;
  *(u32*)(resetWork + 0x18) = 0;
  *(u32*)(resetWork + 0x1C) = 0;
  *(u32*)(resetWork + 0x20) = 0;
  resetWork[9] = 0;
  if (D_80145026 == 0 || D_80145028 == 0) {
    stateRequestedSelectedModes(6);
  } else {
    stateRequestedSelectedModes(0);
  }
  if (D_80146250[0x79] == 0) {
    func_8014D6DC(0xA, 2);
  } else {
    selectActiveWorkSequence(0xB);
  }
  dispatchWork = WORLD00_AREA030_SCRATCH_PTR;
  dispatchWork[3] = (u8)(dispatchWork[3] + 1u);
}
