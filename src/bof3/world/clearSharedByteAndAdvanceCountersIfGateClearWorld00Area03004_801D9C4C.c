#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801D9C4C
 * @behavior When shared halfword 0x80143C40 is zero, clears byte 0x8014419E and
 * increments shared byte 0x80144199 and work byte 3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedByteAndAdvanceCountersIfGateClearWorld00Area03004(void)
{
  u8* work;

  if (D_80143C40 == 0) {
    D_8014419E = 0;
    D_80144199[0]++;
    work = D_1F800044;
    work[3]++;
  }
}
