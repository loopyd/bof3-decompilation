#include "bof3/bof3.h"

extern s8 D_80146872;
extern u32 D_801FE964[];

/* @source 0x801FA294
 * @behavior Dispatches one frame of this overlay's progress chain from the
 * signed shared scenario-progress byte D_80146872: the byte is scaled by four
 * and selects an entry of the overlay-local handler table at 0x801FE964 (15
 * in-image code pointers, 0x801FA2D0 through 0x801FD704), and the selected
 * entry is then called with `jalr` and no arguments. It reads no state other
 * than that byte and writes none; the 0x18-byte frame exists only to keep $ra
 * across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressHandlerScenarioScena0800_801FA294(void) {
  ((void (*)(void))D_801FE964[(s8)D_80146872])();
}
