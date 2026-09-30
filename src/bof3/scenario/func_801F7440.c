#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7440
 * @behavior Dispatches the overlay's second-level per-frame handler from the
 * scratchpad work object: reads the work-object pointer cell at 0x1F800044,
 * takes its dispatch byte at offset 1 and invokes that index of the
 * overlay-local second-level handler table at 0x801FC99C, whose entries 0
 * through 13 are the overlay's own handlers 0x801F7484 through 0x801F82B4; it
 * reads no other state and writes none. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7440(void) {
  D_801FC99C[D_1F800044[1]]();
}
