#include "bof3/bof3.h"

extern s8 D_80146872;

/* Overlay-local scenario state-dispatch table of this code span's handlers,
 * indexed by the signed shared main-RAM scenario state byte D_80146872: its
 * twelve code words run from func_801F7D50 (0x801F7D50, word 0) through
 * func_801FB1B4 (0x801FB1B4, word 11) and end immediately before the data words
 * at 0x801FBF78, so that byte selects which of the overlay's twelve scenario
 * handlers runs this frame. The word immediately before it (0x801FBF44) is zero
 * and terminates the four-word pointer run 0x801FBF34 through 0x801FBF40.
 * @source 0x801FBF48 @kind table
 */
extern void (*D_801FBF48[])(void);

/* @source 0x801F7D14
 * @behavior Per-frame scenario state dispatcher of this overlay: it reads the
 * signed shared main-RAM scenario state byte D_80146872 (0x80146872) with lb,
 * scales it by four and dispatches through the overlay-local handler table at
 * 0x801FBF48, so that byte selects which of the overlay's twelve scenario
 * handlers runs this frame. It reads no state other than that byte and writes
 * none, takes no arguments and returns nothing of its own, and its 0x18-byte
 * frame exists only to hold $ra across the indirect call, with the state-byte
 * read hoisted above the frame setup. No in-image jal targets the address: its
 * only image word is 0x801FBF34, word 0 of the four-word pointer run
 * 0x801FBF34 through 0x801FBF40 that the zero word at 0x801FBF44 terminates and
 * that ends immediately before the table this function indexes, so the overlay
 * reaches this dispatcher only indirectly. Its 60 bytes are
 * instruction-for-instruction identical to the exact sibling
 * dispatchScenarioStateHandlerScenarioScena0500_801F7A78 (0x801F7A78,
 * scena05/00) and to the exact sibling
 * dispatchScenarioStateHandlerScenarioScena0600_801F7298 (0x801F7298), which
 * dispatch the same D_80146872 byte through their own tables at 0x801FD5DC and
 * 0x801FE3D8; they differ only in the table symbol immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchScenarioStateHandlerScenarioScena1300_801F7D14(void) {
  D_801FBF48[D_80146872]();
}
