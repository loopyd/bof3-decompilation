#include "bof3/bof3.h"

/* @source 0x800C1974
 * @behavior Empty overlay handler: returns to the caller immediately without touching any state; the overlay dispatch table references it as an inert slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void noopHandler66(void) {
}
