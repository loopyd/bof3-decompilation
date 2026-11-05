#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD40C
 * @behavior Runs func_8015C088, clears the shared bytes at 0x80146865 and
 * 0x80146866, clears the sub-state byte D_80146875, and requests primary
 * scene state 0x0F through D_80146874. The roles of the first two bytes are
 * unknown. Takes no arguments and does not expose a return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedBytes6566AndRequestPrimaryState0FScenarioScena0200_801FD40C(void) {
  func_8015C088();
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146875 = 0;
  D_80146874 = 0xF;
}
