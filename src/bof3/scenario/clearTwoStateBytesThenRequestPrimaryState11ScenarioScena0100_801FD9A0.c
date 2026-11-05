#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD9A0
 * @behavior Calls the resident helper at 0x8015C088, clears the shared
 * bytes at 0x80146865 and 0x80146866, clears sub-state D_80146875,
 * then requests primary scenario state 0x11 in D_80146874. The helper
 * sets bit 0x40 at 0x80146871 and bit 0x100 at 0x80146258.
 * This callback ignores incoming arguments and has no consumed return
 * value. Its address is entry 15 of the record callback table at 0x801FE308.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearTwoStateBytesThenRequestPrimaryState11ScenarioScena0100_801FD9A0(void) {
  func_8015C088();
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146875 = 0;
  D_80146874 = 0x11;
}
