#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F8748
 * @behavior Dispatches the overlay's second-level per-frame handler from the
 * scratchpad work object: reads the work-object pointer cell at 0x1F800044,
 * takes its dispatch byte at offset 1 and invokes that index of the
 * overlay-local fade-step handler table at 0x801FC9D4, whose entries 0 through
 * 4 are the overlay's own handlers 0x801F878C through 0x801F8AB8; it reads no
 * other state and writes none. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F8748(void) {
  D_801FC9D4[D_1F800044[1]]();
}
