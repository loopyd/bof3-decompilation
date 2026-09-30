#include "bof3/bof3.h"

extern s8 D_80146874;
extern u32 D_801FE970[];

/* @source 0x801FAB5C
 * @behavior Dispatches one frame of this overlay's second-level handler chain
 * from the signed shared byte D_80146874: the byte is scaled by four and
 * selects an entry of the overlay-local handler table at 0x801FE970 (12
 * in-image code pointers, 0x801FAB98 through 0x801FD704), and the selected
 * entry is then called with `jalr` and no arguments. It reads no state other
 * than that byte and writes none; the 0x18-byte frame exists only to keep $ra
 * across the indirect call. This function is itself entry 2 of the outer table
 * at 0x801FE964 that the signed byte D_80146872 indexes (see
 * dispatchProgressHandlerScenarioScena0800_801FA294).
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressHandlerScenarioScena0800_801FAB5C(void) {
  ((void (*)(void))D_801FE970[(s8)D_80146874])();
}
