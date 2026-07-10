#include "internal.h"

extern u32 DAT_80182444[];

/* does: returns one SLUS loader slot base LBA from the EMI table.
 * @source: 0x80162160 FUN_80162160
 */
u32 func_80162160(u32 slot_id) { return DAT_80182444[slot_id]; }
