#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7A18
 * @behavior Dispatches the overlay's second-level per-frame handler from the
 * scratchpad work object: reads the work-object pointer cell at 0x1F800044,
 * takes its dispatch byte at offset 1 and invokes that index of the
 * overlay-local second-level handler table at 0x801FC9AC, whose entries 0
 * through 9 are the overlay's own handlers 0x801F7A5C through 0x801F82B4; it
 * reads no other state and writes none. Takes no arguments and returns
 * nothing; the address is also carried as a code pointer by the data of the
 * concurrently loaded game/00 overlay, so it is an externally referenced
 * entry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7A18(void) {
  D_801FC9AC[D_1F800044[1]]();
}
