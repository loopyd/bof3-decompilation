#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD114
 * @behavior Runs func_8015C088, sets the shared substate byte D_80146875
 * to 2, clears the shared bytes at 0x80146865 and 0x80146866, and requests
 * primary scene state 0x0B through D_80146874. The roles of the cleared
 * bytes remain unknown. Consumes no arguments and exposes no return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedBytes6566AndRequestPrimaryState0BSubstate2ScenarioScena0200_801FD114(void) {
  func_8015C088();
  D_80146875 = 2;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146874 = 0xB;
}
