#include "bof3/core/slus_internal.h"

extern s32 D_8014646C;

/* @behavior copies the current type-0 EMI payload to RAM, then advances the
 * loader's completed-entry count.
 * @source 0x801625E4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void copyEmiType0Payload(void) {
  s32* completed_entries;

  copyEmiTransferChunk();
  completed_entries = &D_8014646C;
  *completed_entries = *completed_entries + 1;
}
