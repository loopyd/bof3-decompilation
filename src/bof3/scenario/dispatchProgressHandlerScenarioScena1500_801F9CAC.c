#include "bof3/bof3.h"

extern s8 D_80146872;
extern u32 D_801FE66C[];

/* @source 0x801F9CAC
 * @behavior Dispatches one frame of this overlay's progress chain from the signed shared
 * scenario-progress byte D_80146872: the byte is scaled by four and selects an entry of the
 * overlay-local handler table at 0x801FE66C (entry 0 = 0x801F9CE8, entry 1 = 0x801F9CFC,
 * entry 2 = 0x801FA998), and the selected entry is then called with `jalr` and no arguments.
 * It reads no state other than that byte and writes none; the 0x18-byte frame exists only to
 * keep $ra across the indirect call. Entries 0 and 1 both store a new value into D_80146872,
 * so the byte selects this frame's handler and that handler rewrites it to select the next one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressHandlerScenarioScena1500_801F9CAC(void) {
  ((void (*)(void))D_801FE66C[(s8)D_80146872])();
}
