#include "bof3/bof3.h"

extern s8 D_80146872;

/* Overlay-local dispatch table of this code span's scenario state handlers,
 * indexed by the shared main-RAM state byte D_80146872: its first entry is
 * clearSharedByteAndAdvanceStateScenarioScena0700_801FA698 (the state-0 handler
 * that advances the state byte to 1) and it runs to entry 11 (0x801FD35C)
 * immediately before the halfword script data at 0x801FDE40.
 * @source 0x801FDE10 @kind table
 */
extern void (*D_801FDE10[])(void);

/* @source 0x801FA65C
 * @behavior Per-frame state dispatcher of this overlay: it reads the signed
 * shared scenario state byte D_80146872, scales it by 4 and dispatches through
 * the pointer table at 0x801FDE10, so the state byte selects which of the
 * overlay's scenario handlers runs this frame. It takes no arguments and
 * returns nothing of its own; the 0x18-byte frame exists only to hold $ra
 * across the indirect call, and the state-byte read is hoisted above the frame
 * setup. No in-image jal targets the address; its only image word is 0x801FDDFC,
 * entry 3 of the eight-word handler row at 0x801FDDF0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchScenarioStateHandlerScenarioScena0700_801FA65C(void) {
  D_801FDE10[D_80146872]();
}
