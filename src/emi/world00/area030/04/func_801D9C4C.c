#include "internal.h"

/**
 * @source 0x801D9C4C
 * @behavior Advances two counters when the area timer is inactive.
 */
void func_801D9C4C(void)
{
  u8* work;

  if (D_80143C40 == 0) {
    D_8014419E = 0;
    D_80144199[0]++;
    work = D_1F800044;
    work[3]++;
  }
}
