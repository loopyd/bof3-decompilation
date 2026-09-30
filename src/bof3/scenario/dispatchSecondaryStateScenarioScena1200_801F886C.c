#include "bof3/bof3.h"

extern s8 D_80146874;

/* Overlay-local second-level state handler table, indexed by the signed shared
 * main-RAM state byte D_80146874: its ten code-pointer words run from the
 * inert noopHandlerScenarioScena1200_801F88A8 (0x801F88A8) through
 * 0x801FB864 and end where the overlay's record-callback table at 0x801FD1EC
 * begins.
 * @source 0x801FD1C4 @kind table
 */
extern u32 D_801FD1C4[];

/* @source 0x801F886C
 * @behavior Overlay-local second-level state dispatcher: it reads the signed
 * shared main-RAM state byte D_80146874 (0x80146874), scales it by four and
 * selects a code-pointer word of the overlay-local handler table at
 * 0x801FD1C4, then invokes the selected entry with `jalr` and no arguments.
 * Its ten entries run from the inert noopHandlerScenarioScena1200_801F88A8
 * (0x801F88A8, entry 0) through 0x801FB864. It reads no state other than that
 * byte and writes none, and the 0x18-byte frame only keeps $ra across the
 * indirect call; the byte read is hoisted above the frame, which is why the
 * original bytes open with the `lui`/`lb` pair on D_80146874 before the
 * `addiu $sp`/`sw $ra` prologue. No in-image `jal` targets the address: the
 * only in-image reference is the pointer word at 0x801FD1C0, entry 2 of the
 * overlay's three-entry progress table at 0x801FD1B8 (0x801F7F64, 0x801F7F78,
 * 0x801F886C), which dispatchProgressHandlerScenarioScena1200_801F7F28
 * (0x801F7F28) selects with the signed shared progress byte D_80146872, so this
 * function runs only while that progress byte reads 2. The name follows the
 * sibling overlays that dispatch the same D_80146874 byte,
 * dispatchSecondaryState_scena16 (0x801F7144) and
 * dispatchSecondaryStateScenarioScena1100_801F7748 (0x801F7748), while the
 * outer D_80146872 dispatcher is named dispatchProgressHandler_* in scena18,
 * scena19 and this target; the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSecondaryStateScenarioScena1200_801F886C(void) {
  ((void (*)(void))D_801FD1C4[(s8)D_80146874])();
}
