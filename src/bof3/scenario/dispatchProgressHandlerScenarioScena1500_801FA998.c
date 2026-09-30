#include "bof3/bof3.h"

extern s8 D_80146874;
extern u32 D_801FE678[];

/* @source 0x801FA998
 * @behavior Dispatches one frame of this overlay's second-level handler chain from the signed
 * shared byte D_80146874: the byte is scaled by four and selects an entry of the overlay-local
 * handler table at 0x801FE678 (entry 0 = 0x801FA9D4, entry 1 = 0x801FA9DC, entry 2 = 0x801FADFC,
 * entry 3 = 0x801FAFA8, entry 4 = 0x801FB788, entry 5 = 0x801FC1D8, entry 6 = 0x801FCBAC), and
 * the selected entry is then called with `jalr` and no arguments. It reads no state other than
 * that byte and writes none; the 0x18-byte frame exists only to keep $ra across the indirect
 * call. This function is itself entry 2 of the outer table at 0x801FE66C that the signed byte
 * D_80146872 indexes (dispatchProgressHandlerScenarioScena1500_801F9CAC), so the outer byte
 * reaches it whenever it reads 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressHandlerScenarioScena1500_801FA998(void) {
  ((void (*)(void))D_801FE678[(s8)D_80146874])();
}
