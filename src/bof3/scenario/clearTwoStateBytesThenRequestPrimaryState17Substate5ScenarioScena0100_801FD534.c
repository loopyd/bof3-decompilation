#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD534
 * @behavior Calls the resident helper at 0x8015C088, sets sub-state byte
 * D_80146875 to 5, clears the shared bytes at 0x80146865 and 0x80146866,
 * then requests primary scenario state 0x17 in D_80146874. Ignores incoming
 * arguments and has no consumed return value. This is entry 3 of the record
 * callback table at 0x801FE308.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearTwoStateBytesThenRequestPrimaryState17Substate5ScenarioScena0100_801FD534(void) {
  func_8015C088();
  D_80146875 = 5;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146874 = 0x17;
}
