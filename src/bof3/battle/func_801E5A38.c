#include "bof3/battle/battle03_internal.h"

/* @source 0x801E5A38
 * @behavior clears the key state bytes across all 0x30 queued-slot entries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E5A38(void) {
  u8 index;

  index = 0u;
  do {
    D_801EC330[index].unk_00 = 0u;
    D_801EC330[index].unk_05 = 0u;
    D_801EC330[index].unk_06 = 0u;
    D_801EC330[index].unk_01 = 0u;
    D_801EC330[index].unk_02 = 0u;
    D_801EC330[index].unk_03 = 0u;
    D_801EC330[index].unk_04 = 0u;
    D_801EC330[index].unk_48 = 0u;
    D_801EC330[index].unk_5d = 0u;
    D_801EC330[index].unk_5e = 0u;
    D_801EC330[index].unk_5f = 0u;
    index += 1u;
  } while (index < 0x30u);
}
