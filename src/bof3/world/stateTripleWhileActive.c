#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E0394
 * @behavior When captured work countdown byte 9 is nonzero, decrements it and
 * reloads the work pointer to add words +0x0C and +0x10 to words +0x34
 * and +0x38, and word +0x14 to halfword +0x3E. Applies these steps
 * even when the decrement reaches zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateTripleWhileActive(void)
{
  u8* countdownWork;
  u8 countdown;

  countdownWork = D_1F800044;
  countdown = countdownWork[9];
  if (countdown != 0) {
    u8* stepWork;

    countdownWork[9] = countdown - 1;
    stepWork = D_1F800044;
    *(s32*)(stepWork + 0x34) += *(s32*)(stepWork + 0x0C);
    *(s32*)(stepWork + 0x38) += *(s32*)(stepWork + 0x10);
    *(u16*)(stepWork + 0x3E) += *(s32*)(stepWork + 0x14);
  }
}
