#include "bof3/bof3.h"

extern s8 D_80146872;

/* Overlay-local outer state-dispatch table at 0x801FE3D8, indexed by the signed
 * shared main-RAM state byte D_80146872: its three code words are
 * seedSharedFieldsThenAdvanceStateScenarioScena0600_801F72D4 (0x801F72D4, the
 * state-0 handler that seeds the shared halfword fields and then advances that
 * byte to 1), func_801F7300 (0x801F7300) and func_801F7A00 (0x801F7A00, this
 * overlay's own primary state dispatcher). It sits at the tail of the code
 * segment's pointer run and ends immediately before the next pointer label
 * D_801FE3E4.
 * @source 0x801FE3D8 @kind table
 */
extern void (*D_801FE3D8[])(void);

/* @source 0x801F7298
 * @behavior Per-frame outer state dispatcher of this overlay: it reads the
 * signed shared main-RAM state byte D_80146872 (0x80146872), scales it by four
 * and dispatches through the overlay-local handler table at 0x801FE3D8, so
 * that byte selects which of the overlay's three outer states runs this frame.
 * It reads no state other than that byte and writes none, takes no arguments
 * and returns nothing of its own, and its 0x18-byte frame exists only to hold
 * $ra across the indirect call, with the state-byte read hoisted above the
 * frame setup. The address is the first boundary of this target's code span
 * (payload offset 1688, the `main` segment start), and its 60 bytes are
 * instruction-for-instruction identical to the exact sibling
 * dispatchScenarioStateHandlerScenarioScena0700_801FA65C (0x801FA65C), which
 * dispatches the same D_80146872 byte through its own table at 0x801FDE10; they
 * differ only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchScenarioStateHandlerScenarioScena0600_801F7298(void) {
  D_801FE3D8[D_80146872]();
}
