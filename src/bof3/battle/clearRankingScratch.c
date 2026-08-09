#include "bof3/battle/battle03_internal.h"

/* @behavior clears one local/enemy ranking scratch set across all battlers.
 * @source 0x801DB494
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearRankingScratch(void) {
  u8 index;

  index = 0u;
  while (index < 3u) {
    D_80145E90[index].unk_119 = 0u;
    D_80145E90[index].unk_124 = 0u;
    index += 1u;
  }

  index = 0u;
  while (index < 8u) {
    D_801EB630[index].unk_f5 = 0u;
    D_801EB630[index].unk_100 = 0u;
    index += 1u;
  }
}
