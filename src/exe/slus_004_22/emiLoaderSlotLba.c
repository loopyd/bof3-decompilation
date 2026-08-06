#include "internal.h"

/* @behavior returns one SLUS loader slot base LBA from the EMI table.
 * @source 0x80162160
 */

/* @source 0x80182444 @kind table */
extern DiscLba emiSlotLbaTable[];

DiscLba emiLoaderSlotLba(EmiLoaderSlotId slot_id) {
  return emiSlotLbaTable[slot_id];
}
