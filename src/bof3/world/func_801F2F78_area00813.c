#include "bof3/world/area00813_internal.h"

/* @source 0x801F2F78
 * @behavior selects area mode 9 when the shared high-bit flag is set; otherwise
 *           clears the shared secondary state byte, reloads the shared halfword
 *           counter and clears it with a countdown restart of 2 when it reads
 *           0xFF, decrements the shared countdown byte, and when the
 *           decremented value is below 2 clears the shared scenario-progress
 *           neighbour byte and selects area mode 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2F78(void)
{
  u16* counter;
  u8   remaining;

  if ((D_80146867 & 0x80u) != 0u) {
    g_areaWork->mode = 9u;
  } else {
    counter = &D_80146876;
    D_80146866 = 0;
    if (*counter == 0xFFu) {
      *counter = 0;
      countdown = 2;
    }
    remaining = (u8)(countdown - 1u);
    countdown = remaining;
    if (remaining < 2u) {
      D_80146865 = 0;
      g_areaWork->mode = 1u;
    }
  }
}
