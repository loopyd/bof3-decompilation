#include "internal.h"

extern DiscLba D_80182444[];

/* @behavior returns one SLUS loader slot base LBA from the EMI table.
 * @source 0x80162160
 */
DiscLba func_80162160(EmiLoaderSlotId slot_id) {
  return D_80182444[slot_id];
}
