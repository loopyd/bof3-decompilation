#include "bof3/bof3.h"

extern s8 D_80146874;
extern u32 D_801FA210[];

/* @source 0x801F863C
 * @behavior Per-frame primary state dispatcher of this overlay: it reads the
 * signed shared main-RAM primary state byte D_80146874 (0x80146874), scales it
 * by four and dispatches through the overlay-local handler table at 0x801FA210,
 * so that byte selects which of this overlay's primary states runs this frame.
 * It reads no state other than that byte and writes none, takes no arguments and
 * returns nothing of its own (the selected handler's $v0 is left in place), and
 * its 0x18-byte frame exists only to hold $ra across the indirect call, with the
 * state-byte read hoisted above the frame setup. No in-image jal targets the
 * address: its only image word is 0x801FA20C, word 2 of the three-word handler
 * run at 0x801FA204 whose other words are the two state handlers
 * advanceStateAndClearSharedByteScenarioScena0400_801F8160 (0x801F8160) and
 * func_801F817C (0x801F817C), so the overlay reaches this dispatcher only
 * indirectly, through the same-overlay chain dispatcher
 * dispatchProgressHandlerScenarioScena0400_801F8124 (0x801F8124) that indexes
 * that run with D_80146872. Its own table at 0x801FA210 holds three code words -
 * the inert entry noopHandlerScenarioScena0400_801F8678 (0x801F8678, word 0),
 * func_801F8680 (word 1) and func_801F8F98 (word 2) - and the word that follows
 * it at 0x801FA21C is 0xFF000100, not a code pointer. Its 60 bytes are
 * instruction-for-instruction identical to the exact siblings
 * dispatchPrimaryStateHandlerScenarioScena0600_801F7A00 (0x801F7A00),
 * dispatchPrimaryStateHandlerScenarioScena0500_801F83EC (0x801F83EC) and the
 * rest of that family, which dispatch the same D_80146874 byte through their own
 * tables; they differ only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPrimaryStateHandlerScenarioScena0400_801F863C(void) {
  ((void (*)(void))D_801FA210[(s8)D_80146874])();
}
