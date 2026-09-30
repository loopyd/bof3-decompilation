#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD574
 * @behavior Runs the shared front-end startup helper func_8015C088 and then
 * requests primary scene state 0x17 by storing 0x17 in the main-RAM state byte
 * D_80146874, clearing the secondary sub-state byte D_80146875 first; it takes
 * no arguments and returns nothing, and its 0x18-byte frame only keeps $ra
 * across the call. No in-image jal targets the address; its only image
 * reference is the word at 0x801FE358, entry 21 of the overlay callback table
 * D_801FE304 that func_801FD084 indexes with a record's byte field 0x7A. The
 * neighbouring primary-state table D_801FE294 is indexed by D_80146874 itself
 * (func_801F7A60), so the 0x17 store selects that table's entry 0x17.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState17AndClearSubstateScenarioScena0200_801FD574(void) {
  func_8015C088();
  D_80146875 = 0;
  D_80146874 = 0x17;
}
