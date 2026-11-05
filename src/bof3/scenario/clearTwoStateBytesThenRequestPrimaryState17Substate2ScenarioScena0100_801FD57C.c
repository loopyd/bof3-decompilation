#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD57C
 * @behavior Calls the resident helper at 0x8015C088, sets sub-state byte
 * D_80146875 to 2, clears the shared bytes at 0x80146865 and 0x80146866,
 * then requests primary scenario state 0x17 in D_80146874. Ignores incoming
 * arguments and has no consumed return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearTwoStateBytesThenRequestPrimaryState17Substate2ScenarioScena0100_801FD57C(void) {
  func_8015C088();
  D_80146875 = 2;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146874 = 0x17;
}
