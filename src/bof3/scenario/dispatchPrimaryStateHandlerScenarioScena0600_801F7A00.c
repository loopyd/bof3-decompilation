#include "bof3/bof3.h"

extern s8 D_80146874;

/* Overlay-local primary state-dispatch table at 0x801FE3E4, indexed by the
 * signed shared main-RAM state byte D_80146874: its eighteen code words run
 * from the inert entry noopHandlerScenarioScena0600_801F7A3C (0x801F7A3C,
 * word 0) to func_801FC718 (0x801FC718, word 17), so the byte selects which of
 * this overlay's scenario handlers runs this frame. It begins immediately after
 * the three-word label D_801FE3D8 and ends immediately before the next pointer
 * label D_801FE42C.
 * @source 0x801FE3E4 @kind table
 */
extern void (*D_801FE3E4[])(void);

/* @source 0x801F7A00
 * @behavior Per-frame primary state dispatcher of this overlay: it reads the
 * signed shared main-RAM state byte D_80146874 (0x80146874), scales it by four
 * and dispatches through the overlay-local handler table at 0x801FE3E4, so
 * that byte selects which of the overlay's eighteen primary states runs this
 * frame. It reads no state other than that byte and writes none, takes no
 * arguments and returns nothing of its own, and its 0x18-byte frame exists only
 * to hold $ra across the indirect call, with the state-byte read hoisted above
 * the frame setup. No in-image jal targets the address: its only image word is
 * 0x801F7A00 itself, word 2 of the three-word label D_801FE3D8, so the overlay
 * reaches it only indirectly, through the D_80146872 dispatcher
 * dispatchScenarioStateHandlerScenarioScena0600_801F7298 (0x801F7298). Its 60
 * bytes are instruction-for-instruction identical to that dispatcher and to the
 * exact sibling dispatchPrimaryStateHandlerScenarioScena0700_801FAA2C
 * (0x801FAA2C), which dispatches the same D_80146874 byte through its own table
 * at 0x801FDE1C; they differ only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPrimaryStateHandlerScenarioScena0600_801F7A00(void) {
  D_801FE3E4[D_80146874]();
}
