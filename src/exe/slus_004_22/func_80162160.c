#include "internal.h"

extern DiscLba DAT_80182444[];

/* @behavior returns one SLUS loader slot base LBA from the EMI table.
 * @source 0x80162160 FUN_80162160
 */
DiscLba func_80162160(EmiLoaderSlotId slot_id) {
  return DAT_80182444[slot_id];
}
