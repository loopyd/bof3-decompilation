#include "bof3/bof3.h"

extern s8 D_80146872;
extern u32 D_801FAEF0[];

/* @source 0x801F7288
 * @behavior Dispatches one frame of this overlay's progress chain from the
 * signed shared scenario state byte D_80146872 (0x80146872): the byte is
 * scaled by four and selects a word of the overlay-local code-pointer run at
 * 0x801FAEF0 (its first words are 0x801F72C4, 0x801F72E4 and 0x801F7748), and
 * the selected entry is then invoked with `jalr` and no arguments. It reads no
 * state other than that byte and writes none; the 0x18-byte frame exists only
 * to keep $ra across the indirect call. The address is word 0 of the
 * code-pointer run at 0x801FAEDC (0x801FAEDC holds 0x801F7288), so the overlay
 * reaches it indirectly and no in-image jal targets it. Entry 0 of its table
 * (armEffectBankAndAdvanceStateScenarioScena1100_801F72C4, 0x801F72C4) stores 1
 * into D_80146872 and entry 1 (func_801F72E4, 0x801F72E4) ends by storing 2
 * into it, so the byte walks 0 -> 1 -> 2 through a state chain. The 60 bytes
 * are instruction-for-instruction identical to the exact progress dispatchers of
 * the sibling EMI scenario overlays (dispatchProgressHandler_scena18 0x801F6C04,
 * dispatchProgressHandlerScenarioScena0400_801F8124 0x801F8124,
 * dispatchProgressHandlerScenarioScena0800_801FA294 0x801FA294), which differ
 * only in the table address; the address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressHandlerScenarioScena1100_801F7288(void) {
  ((void (*)(void))D_801FAEF0[(s8)D_80146872])();
}
