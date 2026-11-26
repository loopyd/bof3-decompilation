#include "bof3/bof3.h"

/* @source 0x800C19CC
 * @behavior Empty overlay handler: returns to the caller immediately without touching any state; the overlay dispatch table references it as an inert slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void noopHandler156(void) {
}
