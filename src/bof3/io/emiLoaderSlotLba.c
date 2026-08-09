#include "bof3/core/slus_internal.h"

/* @behavior returns one SLUS loader slot base LBA from the EMI table.
 * @source 0x80162160
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */

/* @source 0x80182444 @kind table */
extern DiscLba emiSlotLbaTable[];

DiscLba emiLoaderSlotLba(EmiLoaderSlotId slot_id) {
  return emiSlotLbaTable[slot_id];
}
