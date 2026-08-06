#include "internal.h"

/* possible name: emi_build_entry_lba_table
 * @behavior derives per-entry LBAs from the base header LBA and TOC sizes.
 */
void buildEmiEntryLbas(u32 base_lba, const EmiTocEntry* entries,
                          size_t entry_count, u32* entry_lbas) {
  size_t index;

  if (entries == NULL || entry_lbas == NULL || entry_count == 0) {
    return;
  }

  entry_lbas[0] = base_lba + 1u;

  for (index = 1; index < entry_count; ++index) {
    u32 prev_size = entries[index - 1].size;
    entry_lbas[index] = entry_lbas[index - 1] + ((prev_size + 0x7ffu) >> 11);
  }
}
