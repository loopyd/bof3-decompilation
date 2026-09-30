#include "bof3/bof3.h"

extern s8 D_80146874;

/* Overlay-local primary state-dispatch table of this code span's scenario
 * handlers, indexed by the signed shared primary state byte D_80146874: its
 * thirty-two code words run from the inert entry
 * noopHandlerScenarioScena0500_801F8428 (0x801F8428, word 0) to func_801FCD0C
 * (0x801FCD0C, word 31), so that byte selects which of the overlay's primary
 * states runs this frame. It begins immediately after the last word of the
 * three-word state row at 0x801FD5DC, and the word immediately after its last
 * entry (0x801FD668) is not a code pointer.
 * @source 0x801FD5E8 @kind table
 */
extern void (*D_801FD5E8[])(void);

/* @source 0x801F83EC
 * @behavior Per-frame primary state dispatcher of this overlay: it reads the
 * signed shared primary state byte D_80146874 (0x80146874), scales it by four
 * and dispatches through the overlay-local handler table at 0x801FD5E8, so that
 * byte selects which of the overlay's primary states runs this frame. It reads
 * no state other than that byte and writes none, takes no arguments and returns
 * nothing of its own, and its 0x18-byte frame exists only to hold $ra across the
 * indirect call, with the state-byte read hoisted above the frame setup. No
 * in-image jal targets the address: its only image word is 0x801FD5E4, word 2 of
 * the three-word state row at 0x801FD5DC, so the overlay reaches this dispatcher
 * only indirectly, through the same-overlay state dispatcher
 * dispatchScenarioStateHandlerScenarioScena0500_801F7A78 (0x801F7A78) that
 * indexes that row with D_80146872. Its 60 bytes are instruction-for-instruction
 * identical to that dispatcher and to the exact sibling
 * dispatchPrimaryStateHandlerScenarioScena0600_801F7A00 (0x801F7A00), which
 * dispatches the same D_80146874 byte through its own table at 0x801FE3E4; they
 * differ only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPrimaryStateHandlerScenarioScena0500_801F83EC(void) {
  D_801FD5E8[D_80146874]();
}
