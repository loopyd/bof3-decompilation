#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D3798
 * @behavior when the phase byte D_80143BB0 is not 2, scans the D_80146254
 *           0x140-byte-stride records at D_80145FCC and clears the handler
 *           index D_801D4286 as soon as one of them selects a 164-byte record
 *           whose owner byte in D_80144983 equals masterIndex; only when no
 *           visible record belongs to the selected master does it start that
 *           master's +9 action through func_80150224, write the phase byte as 2
 *           and advance the handler index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3798(void) {
  s32 index;

  if (D_80143BB0 == 2) {
    return;
  }
  for (index = 0; index < D_80146254; index++) {
    s32 offset;
    s32 record_offset;

    offset = index * 0x140;
    record_offset = D_80145FCC[offset] * 164;
    if (D_80144983[record_offset] == masterIndex) {
      break;
    }
  }
  if (index < D_80146254) {
    D_801D4286 = 0;
    return;
  }
  func_80150224((s16)(masterActionBaseTable[masterIndex] + 9));
  D_80143BB0 = 2;
  D_801D4286 = D_801D4286 + 1;
}
