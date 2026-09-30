#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F70F0
 * @behavior Dispatches the overlay's per-frame handler for the work object's
 * later states from the scratchpad work object: reads the work-object pointer
 * cell at 0x1F800044, takes its state byte at offset 1 and invokes entry
 * (state + 2) of the overlay-local handler table at 0x801FC980, whose handler
 * word at 0x801FC988 is the 0x801F7264 handler and so this dispatcher's entry
 * 0; it reads no other state and writes none. The 0x801F6DC0 dispatcher
 * indexes the same table from its first entry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Handler_801F70F0(void) {
  D_801FC980[D_1F800044[1] + 2]();
}
