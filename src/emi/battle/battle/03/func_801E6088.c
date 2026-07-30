#include "internal.h"

/* @source 0x801E6088 */
/* @behavior Decrements the timer; when its post-decrement value is zero, increments the selector and resets the timer to 4. */
void func_801E6088(void) {
  if (D_801EC2E0->unk_09-- == 0) {
    D_801EC2E0->unk_01++;
    D_801EC2E0->unk_09 = 4;
  }
}
