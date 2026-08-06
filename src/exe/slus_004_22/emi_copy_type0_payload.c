#include "internal.h"

extern s32 D_8014646C;

/* @behavior copies the current type-0 EMI payload to RAM, then advances the
 * loader's completed-entry count.
 * @source 0x801625E4
 */
void emi_copy_type0_payload(void) {
  s32* completed_entries;

  emi_copy_transfer_chunk();
  completed_entries = &D_8014646C;
  *completed_entries = *completed_entries + 1;
}
