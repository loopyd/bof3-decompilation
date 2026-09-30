#include "bof3/bof3.h"

extern s8 D_80146874;
extern u32 D_801FE2A8[];

/* @source 0x801F8AA0
 * @behavior Dispatches this overlay's per-frame handler named by the signed progress
 * byte D_80146874 (0x80146874, the shared secondary scenario progress byte) through the
 * overlay's own handler table at 0x801FE2A8 (the second table inside the pointer block
 * whose first table starts at 0x801FE29C), invoking the selected entry with no arguments
 * and returning whatever it returns; it reads no other state and writes none.
 * Byte-identical to the exact sibling dispatchProgressHandler_scena18_801F6C68
 * (0x801F6C68, emi/scenario/scena18/00), which indexes the same progress byte through
 * that overlay's own table at 0x801F6D78 (0x801F6D6C + 12).
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressHandlerScenarioScena0100_801F8AA0(void) {
  ((void (*)(void))D_801FE2A8[(s8)D_80146874])();
}
