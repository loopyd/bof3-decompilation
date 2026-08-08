#include "internal.h"

/**
 * @source 0x801D9C9C
 * @behavior Advances the shared counters and clears work byte 3 in mode 4.
 */
void func_801D9C9C(void)
{
  u8* work;

  if (D_80144199[0] == 4) {
    work = D_1F800044;
    D_8014403D++;
    work[2]++;
    D_1F800044[3] = 0;
  }
}
