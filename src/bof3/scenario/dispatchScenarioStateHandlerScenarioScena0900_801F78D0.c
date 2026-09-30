#include "bof3/bof3.h"

extern s8 D_80146872;

/* Overlay-local dispatch table of this code span's scenario state handlers,
 * indexed by the signed shared main-RAM state byte D_80146872: its first entry
 * is setScenarioScena0900_801F790C (0x801F790C, the state-0 handler that
 * advances the state byte to 1), the following 19 words are in-image code
 * pointers as well, and the word at 0x801FE430 (0xFF000100) is not a pointer.
 * @source 0x801FE3E0 @kind table
 */
extern void (*D_801FE3E0[])(void);

/* @source 0x801F78D0
 * @behavior Per-frame state dispatcher of this overlay: it reads the signed
 * shared main-RAM state byte D_80146872, scales it by four and dispatches
 * through the pointer table at 0x801FE3E0, so that state byte selects which of
 * the overlay's scenario handlers runs this frame. It takes no arguments and
 * returns nothing of its own; the 0x18-byte frame exists only to hold $ra
 * across the indirect call, and the state-byte read is hoisted above the frame
 * setup. No in-image jal targets the address; its only image word is the one at
 * 0x801FE3CC, the first slot of the five-word pointer run 0x801FE3CC..0x801FE3DC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchScenarioStateHandlerScenarioScena0900_801F78D0(void) {
  D_801FE3E0[D_80146872]();
}
