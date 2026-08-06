#include "internal.h"

extern DiscLba emiSlotLbaTable[]; /* @kind: table */

/* @behavior returns one SLUS loader slot base LBA from the EMI table.
 * @source 0x80162160
 */
DiscLba emi_loader_slot_lba(EmiLoaderSlotId slot_id) {
  return emiSlotLbaTable[slot_id];
}
