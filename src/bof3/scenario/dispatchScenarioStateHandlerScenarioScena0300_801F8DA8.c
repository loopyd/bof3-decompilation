#include "bof3/bof3.h"

extern s8 D_80146872;

/* Overlay-local dispatch table of this code span's scenario state handlers,
 * indexed by the signed shared main-RAM state byte D_80146872: it is the
 * three-pointer run at 0x801FCFD4 through 0x801FCFDC whose entry 0 is
 * advanceStateAndClearSharedByteScenarioScena0300_801F8DE4 (0x801F8DE4, the
 * state-0 handler that writes 1 to that byte), entry 1 is 0x801F8E00 and entry
 * 2 is 0x801F92F4. It sits at the end of the data run that the
 * func_801FCF24 boundary still covers and ends immediately before the
 * noopHandlerScenarioScena0300_801F9330 pointer word at 0x801FCFE0.
 * @source 0x801FCFD4 @kind table
 */
extern void (*D_801FCFD4[])(void);

/* @source 0x801F8DA8
 * @behavior Per-frame state dispatcher of this overlay: it reads the signed
 * shared scenario state byte D_80146872, scales it by four and dispatches
 * through the overlay-local pointer table at 0x801FCFD4, so the state byte
 * selects which of the overlay's scenario handlers runs this frame. It reads
 * no state other than that byte and writes none; it takes no arguments and
 * returns nothing of its own, the 0x18-byte frame exists only to hold $ra
 * across the indirect call, and the state-byte read is hoisted above the frame
 * setup. No in-image jal targets the address; its only image word is 0x801F8DA8
 * at 0x801FCFC0, word 5 of the ten-word pointer run at 0x801FCFAC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchScenarioStateHandlerScenarioScena0300_801F8DA8(void) {
  D_801FCFD4[D_80146872]();
}
