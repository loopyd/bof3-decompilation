#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FDA2C
 * @behavior Calls the shared helper at 0x8015C088, then requests primary
 * scenario state 0x0B with sub-state 0x0F by storing 0x0F to D_80146875
 * followed by 0x0B to D_80146874. The helper sets bit 0x40 in the byte at
 * 0x80146871 and bit 0x100 in the halfword at 0x80146258. This callback
 * ignores incoming arguments and has no consumed return value. Its address
 * occupies entry 17 of the record callback table at 0x801FE308.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState0BSubstate0FScenarioScena0100_801FDA2C(void) {
  func_8015C088();
  D_80146875 = 0xF;
  D_80146874 = 0xB;
}
