#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F9BD4
 * @behavior Empty primary-state handler: takes no arguments, reads and writes
 * no state and returns immediately. Its address is entry 3 of the overlay-local
 * handler table D_801FCA10 and, equivalently, entry 0 of the secondary table
 * D_801FCA1C, so primary state 0 of the state dispatcher func_801F9B98 has no
 * per-frame work.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F9BD4(void) {
}
