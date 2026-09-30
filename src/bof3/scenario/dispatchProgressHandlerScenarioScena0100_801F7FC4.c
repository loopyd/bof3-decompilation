#include "bof3/bof3.h"

extern s8 D_80146872;
extern u32 D_801FE29C[];

/* @source 0x801F7FC4
 * @behavior Dispatches this overlay's per-frame handler named by the signed progress
 * byte D_80146872 (0x80146872, the shared scenario progress byte) through the overlay's
 * own 24-entry handler table at 0x801FE29C, invoking the selected entry with no
 * arguments and returning whatever it returns; it reads no other state and writes none.
 * Byte-identical to the exact sibling dispatchProgressHandler_scena18 (0x801F6C04,
 * emi/scenario/scena18/00), which indexes the same progress byte through that overlay's
 * own table at 0x801F6D6C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressHandlerScenarioScena0100_801F7FC4(void) {
  ((void (*)(void))D_801FE29C[(s8)D_80146872])();
}
