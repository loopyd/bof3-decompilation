#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FB3DC
 * @behavior Runs the shared front-end startup helper func_8015C088
 * (0x8015C088) and then requests primary scene state 6 with secondary
 * sub-state 0x19 by storing 6 in the shared primary state byte D_80146874
 * (0x80146874) and 0x19 in the shared secondary sub-state byte D_80146875
 * (0x80146875). Takes no arguments and returns nothing; its 0x18-byte frame
 * only keeps $ra across the call, and each byte store materialises the 0x8014
 * page in $at on its own, which is how the psyq compiler emits separate
 * shared-byte stores. No in-image jal targets the address: its only image word
 * is 0x801FBFC0, entry 2 of the thirteen-entry null-terminated pointer run at
 * 0x801FBFB8 (0x801FBFB8 through 0x801FBFE8), the same table whose entry 0 is
 * the exact sibling incrementWorkHalfword7EScenarioScena1300_801FB390
 * (0x801FB390) and whose entry 1 is
 * requestPrimaryState3Substate5ScenarioScena1300_801FB3A4 (0x801FB3A4) in this
 * overlay. The 0x38 bytes repeat the exact call-then-two-store shape of the
 * sibling lift requestPrimaryState1ASubstate19ScenarioScena0200_801FD684
 * (0x801FD684, scena02/00), differing only in the two stored constants.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState6Substate19ScenarioScena1300_801FB3DC(void) {
  func_8015C088();
  D_80146874 = 6;
  D_80146875 = 0x19;
}
