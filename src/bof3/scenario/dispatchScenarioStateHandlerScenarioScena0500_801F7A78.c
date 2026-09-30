#include "bof3/bof3.h"

extern s8 D_80146872;

/* Overlay-local state-dispatch table of this code span's scenario handlers,
 * indexed by the signed shared main-RAM state byte D_80146872: its three code
 * words are setScenarioScena0500_801F7AB4 (0x801F7AB4, the state-0 handler that
 * advances the state byte to 1), func_801F7AC8 (0x801F7AC8) and
 * dispatchPrimaryStateHandlerScenarioScena0500_801F83EC (0x801F83EC, this
 * overlay's own primary state dispatcher). The two words immediately before it
 * (0x801FD5C0 and 0x801FD5C4) are not code pointers.
 * @source 0x801FD5DC @kind table
 */
extern void (*D_801FD5DC[])(void);

/* @source 0x801F7A78
 * @behavior Per-frame state dispatcher of this overlay: it reads the signed
 * shared main-RAM state byte D_80146872 (0x80146872), scales it by four and
 * dispatches through the overlay-local handler table at 0x801FD5DC, so that
 * byte selects which of the overlay's three states runs this frame. It reads no
 * state other than that byte and writes none, takes no arguments and returns
 * nothing of its own, and its 0x18-byte frame exists only to hold $ra across the
 * indirect call, with the state-byte read hoisted above the frame setup. The
 * address's only image word is 0x801FD5C8, word 0 of the five-word pointer run
 * 0x801FD5C8..0x801FD5D8 that ends immediately before that table, so the overlay
 * reaches this dispatcher only indirectly. Its 60 bytes are
 * instruction-for-instruction identical to the exact sibling
 * dispatchScenarioStateHandlerScenarioScena0600_801F7298 (0x801F7298), which
 * dispatches the same D_80146872 byte through its own table at 0x801FE3D8; they
 * differ only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchScenarioStateHandlerScenarioScena0500_801F7A78(void) {
  D_801FD5DC[D_80146872]();
}
