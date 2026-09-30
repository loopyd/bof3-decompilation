#include "bof3/world/area01613_internal.h"

/* @source 0x801F2C04
 * @behavior writes 2 to the shared mode byte at 0x801448EC when the shared
 * gate byte at 0x801490C7 is zero, and stores -1 into the shared status
 * halfword at 0x801490A8 on every path.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2C04(void) {
  if (D_801490C7 == 0) {
    D_801448EC[0] = 2;
    D_801490A8 = 0xFFFFu;
  } else {
    D_801490A8 = 0xFFFFu;
  }
}
