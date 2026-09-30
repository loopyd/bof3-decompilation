#include "bof3/bof3.h"

extern s8 D_80146872;

/* Overlay-local dispatch table of this code span's scenario state handlers,
 * indexed by the shared main-RAM state byte D_80146872: it is the thirteen-word
 * run at 0x801FC418 through 0x801FC448 whose first entry is
 * setScenarioScena1400_801F7014 (0x801F7014, the state-0 handler that stores 1
 * in that byte) and whose last entry is noopHandlerScenarioScena1400_801FB374
 * (0x801FB374); entry 3 and entry 11 are both the inert handler
 * noopHandlerScenarioScena1400_801F7B98 (0x801F7B98). It ends immediately
 * before the data word at 0x801FC44C.
 * @source 0x801FC418 @kind table
 */
extern void (*D_801FC418[])(void);

/* @source 0x801F6FD8
 * @behavior Per-frame state dispatcher of this overlay: it reads the signed
 * shared scenario state byte D_80146872, scales it by four and dispatches
 * through the pointer table at 0x801FC418, so the state byte selects which of
 * the overlay's scenario handlers runs this frame. It reads no state other
 * than that byte and writes none; it takes no arguments and returns nothing of
 * its own, and the 0x18-byte frame exists only to hold $ra across the indirect
 * call, with the state-byte read hoisted above the frame setup. No in-image jal
 * targets the address; its only image word is 0x801F6FD8 itself, word 0 of the
 * five-word pointer run at 0x801FC404 (0x801F6FD8 / 0x801FB37C / 0x801FB648 /
 * 0x801FB84C / 0) that no code in this target reaches directly.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchScenarioStateHandlerScenarioScena1400_801F6FD8(void) {
  D_801FC418[D_80146872]();
}
