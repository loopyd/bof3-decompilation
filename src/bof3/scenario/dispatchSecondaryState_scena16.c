#include "bof3/scenario/scena16_internal.h"

/* @behavior dispatches through the secondary SCENA16 state table: it loads the
 * signed front-end state byte D_80146874 with a signed byte load, scales it by
 * four onto the overlay-local handler table secondaryStateTable (0x801F8558) and
 * calls the selected handler with `jalr` and no arguments, returning nothing of
 * its own. It reads no other memory and writes none; the 0x18-byte frame exists
 * only to keep $ra across the indirect call, and the state-byte read is hoisted
 * above the frame setup while the table load and the `jalr` keep their delay-slot
 * nops.
 * @source 0x801F7144
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSecondaryState(void) {
  ((void (*)(void))secondaryStateTable[(s8)D_80146874])();
}
