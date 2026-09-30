#include "bof3/bof3.h"

extern s8 D_80146874;

/* Overlay-local primary state-dispatch table of this code span's handlers,
 * indexed by the signed shared primary state byte D_80146874: its seventeen code
 * words run from the inert entry noopHandlerScenarioScena0900_801F8414
 * (0x801F8414, word 0) to func_801FBC78 (0x801FBC78, word 16), so the byte
 * selects which of the overlay's scenario handlers runs this frame. It begins
 * three words into the twenty-word state-handler table at 0x801FE3E0 and ends
 * immediately before the non-pointer word 0xFF000100 at 0x801FE430.
 * @source 0x801FE3EC @kind table
 */
extern void (*D_801FE3EC[])(void);

/* @source 0x801F83D8
 * @behavior Per-frame primary state dispatcher of this overlay: it reads the
 * signed shared primary state byte D_80146874 (0x80146874), scales it by four
 * and dispatches through the overlay-local handler table at 0x801FE3EC, so that
 * byte selects which of the overlay's seventeen primary states runs this frame.
 * It reads no state other than that byte and writes none, takes no arguments and
 * returns nothing of its own, and its 0x18-byte frame exists only to hold $ra
 * across the indirect call, with the state-byte read hoisted above the frame
 * setup. No in-image jal targets the address: its only image word is 0x801FE3E8,
 * word 2 of the twenty-word state-handler table at 0x801FE3E0 whose entry 0 is
 * setScenarioScena0900_801F790C and whose word at 0x801FE430 (0xFF000100) is not
 * a pointer, so the overlay reaches this dispatcher only indirectly, through the
 * same-overlay state dispatcher dispatchScenarioStateHandlerScenarioScena0900_801F78D0
 * (0x801F78D0) that indexes that table with D_80146872. Its 60 bytes are
 * instruction-for-instruction identical to that dispatcher and to the exact
 * sibling dispatchPrimaryStateHandlerScenarioScena0600_801F7A00 (0x801F7A00),
 * which dispatches the same D_80146874 byte through its own table at
 * 0x801FE3E4; they differ only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPrimaryStateHandlerScenarioScena0900_801F83D8(void) {
  D_801FE3EC[D_80146874]();
}
