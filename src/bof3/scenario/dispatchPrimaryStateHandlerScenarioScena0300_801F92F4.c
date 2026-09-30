#include "bof3/bof3.h"

extern s8 D_80146874;

/* Overlay-local primary-state handler table at 0x801FCFE0, indexed by the
 * signed shared main-RAM state byte D_80146874: its nine code-pointer words run
 * from the inert entry noopHandlerScenarioScena0300_801F9330 (0x801F9330,
 * word 0) through func_801FB8A4 (0x801FB8A4, word 8). It begins immediately
 * after the three-word run at 0x801FCFD4 ({0x801F8DE4, 0x801F8E00,
 * 0x801F92F4}, the run the exact scenario-state dispatcher indexes with
 * D_80146872) and ends immediately before the two data words
 * 0x000100FF/0x000001FF at 0x801FD004/0x801FD008.
 * @source 0x801FCFE0 @kind table
 */
extern void (*D_801FCFE0[])(void);

/* @source 0x801F92F4
 * @behavior Per-frame primary state dispatcher of this overlay: it reads the
 * signed shared main-RAM primary state byte D_80146874 (0x80146874) with `lb`,
 * scales it by four and dispatches through the overlay-local handler table at
 * 0x801FCFE0, so that byte selects which of the overlay's primary-state entries
 * runs this frame (slots 0 and 6 are the exact handlers
 * noopHandlerScenarioScena0300_801F9330 and func_801FC690, the latter being
 * reached through the record dispatcher to request state 6). It reads no state
 * other than that byte and writes none, takes no arguments and returns nothing
 * of its own, and its 0x18-byte frame exists only to hold $ra across the
 * indirect call, with the state-byte read hoisted above the frame setup. No
 * in-image jal targets the address: its only image word is 0x801FCFDC, entry 2
 * of the three-word pointer run at 0x801FCFD4
 * (advanceStateAndClearSharedByteScenarioScena0300_801F8DE4 / func_801F8E00 /
 * 0x801F92F4) that the exact overlay state dispatcher
 * dispatchScenarioStateHandlerScenarioScena0300_801F8DA8 (0x801F8DA8) indexes
 * with D_80146872, so the overlay reaches this function only indirectly. Its
 * 60 bytes are instruction-for-instruction identical to that dispatcher and to
 * the D_80146874 dispatchers dispatchPrimaryStateHandlerScenarioScena0600_801F7A00
 * (0x801F7A00), dispatchPrimaryStateHandlerScenarioScena0700_801FAA2C
 * (0x801FAA2C) and dispatchPrimaryStateHandlerScenarioScena1400_801F7B5C
 * (0x801F7B5C), differing only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPrimaryStateHandlerScenarioScena0300_801F92F4(void) {
  D_801FCFE0[D_80146874]();
}
