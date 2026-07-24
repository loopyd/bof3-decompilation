#include "internal.h"

/* @stores 0x-14 to offset 2 of the pointer loaded from D_801463A0.
 * @behavior stores signed halfword -20 at byte offset 4 through D_801463A0
 * @source 0x8009DE50
 */
void func_8009DE50(void) {
  ((s16*)D_801463A0)[2] = -20;
}
