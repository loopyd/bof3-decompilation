#include "bof3/battle/battle03_internal.h"

/* @source 0x801DD760
 * @behavior Selects the local work array for selectors below three and the alternate region otherwise.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8* selectWorkRecordBase(u8 arg0) {
  if (arg0 < 3u) {
    return (u8*)&D_80145E90[arg0];
  }

  return D_801EB2E8 + ((arg0 * 36u - arg0) * 8u);
}
