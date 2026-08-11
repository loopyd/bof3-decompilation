#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801C3204
 * @behavior Initializes two input hold-delay bytes from the current indexed record flags.
 */
void initializeInputHoldDelays(void)
{
  u8* work;
  u8 index;

  work = D_80146250;
  index = work[0x13C];
  if (D_80144974[index].flags & 0x80) {
    work[0x118] = 10;
  }

  work = D_80146250;
  index = work[0x13C];
  if (D_80144974[index].flags & 0x20) {
    work[0x119] = 40;
  }
}
