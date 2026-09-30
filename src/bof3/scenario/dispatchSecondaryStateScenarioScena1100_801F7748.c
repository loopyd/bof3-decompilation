#include "bof3/bof3.h"

extern s8 D_80146874;
extern u32 D_801FAEFC[];

/* @source 0x801F7748
 * @behavior Dispatches one frame of this overlay's second-level handler chain from the signed
 * shared byte D_80146874: the byte is scaled by four and selects a word of the overlay-local
 * handler run at 0x801FAEFC (its ten entries are noopHandlerScenarioScena1100_801F7784,
 * func_801F778C, func_801F7BFC, func_801F7CC8, func_801F7F08, func_801F83C8, func_801F8B84,
 * noopHandlerScenarioScena1100_801F917C, func_801F9184 and func_801FA3A4), and the selected
 * entry is then invoked with `jalr` and no arguments. It reads no state other than that byte and
 * writes none; the 0x18-byte frame exists only to keep $ra across the indirect call. Word 2 of
 * the overlay's outer progress table at 0x801FAEF0 is this same address, so
 * dispatchProgressHandlerScenarioScena1100_801F7288 reaches it whenever the shared byte
 * D_80146872 reads 2, and this dispatcher then walks the D_80146874 second-level chain. The 60 bytes
 * are instruction-for-instruction identical to the exact second-level dispatcher of the sibling
 * EMI scenario overlay SCENA15 (dispatchProgressHandlerScenarioScena1500_801FA998 0x801FA998),
 * which differs only in the table address; the address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSecondaryStateScenarioScena1100_801F7748(void) {
  ((void (*)(void))D_801FAEFC[(s8)D_80146874])();
}
