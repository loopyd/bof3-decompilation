#include "bof3/bof3.h"

extern s8 D_80146874;

/* Overlay-local primary-state handler table at 0x801FC424, indexed by the
 * signed shared main-RAM state byte D_80146874: its ten code-pointer words run
 * from the inert entry noopHandlerScenarioScena1400_801F7B98 (0x801F7B98,
 * word 0) through noopHandlerScenarioScena1400_801FB374 (0x801FB374, word 9),
 * word 8 being noopHandlerScenarioScena1400_801F7B98 again. It begins
 * immediately after the three-word run at 0x801FC418 and ends immediately
 * before the data word 0xFF000100 at 0x801FC44C.
 * @source 0x801FC424 @kind table
 */
extern void (*D_801FC424[])(void);

/* @source 0x801F7B5C
 * @behavior Per-frame primary state dispatcher of this overlay: it reads the
 * signed shared main-RAM state byte D_80146874 (0x80146874) with `lb`, scales
 * it by four and dispatches through the overlay-local handler table at
 * 0x801FC424, so that byte selects which of the overlay's primary-state
 * entries runs this frame. It reads no state other than that byte and writes
 * none, takes no arguments and returns nothing of its own, and its 0x18-byte
 * frame exists only to hold $ra across the indirect call, with the state-byte
 * read hoisted above the frame setup. No in-image jal targets the address: its
 * only image word is 0x801FC420, entry 2 of the three-word pointer run at
 * 0x801FC418 (setScenarioScena1400_801F7014 / func_801F7028 / 0x801F7B5C),
 * which the scenario-state dispatcher
 * dispatchScenarioStateHandlerScenarioScena1400_801F6FD8 (0x801F6FD8) indexes
 * with D_80146872, so the overlay reaches this function only indirectly. Its
 * 60 bytes are instruction-for-instruction identical to that dispatcher and to
 * the D_80146874 dispatchers dispatchPrimaryStateHandlerScenarioScena0600_801F7A00
 * (0x801F7A00) and dispatchPrimaryStateHandlerScenarioScena0700_801FAA2C
 * (0x801FAA2C), differing only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPrimaryStateHandlerScenarioScena1400_801F7B5C(void) {
  D_801FC424[D_80146874]();
}
