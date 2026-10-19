#include "bof3/ui/shop00_internal.h"

/* @source 0x801D6CE4
 * @behavior Copies the seven 32-bit words at state byte offset 0xCA0 into
 *           record index of the overlay-local panel table D_801E6098.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D6CE4(u8 index, u8* state) {
  D_801E6098[index] = FIELD_REF(ShopPanelWords, state, 0xCA0);
}
