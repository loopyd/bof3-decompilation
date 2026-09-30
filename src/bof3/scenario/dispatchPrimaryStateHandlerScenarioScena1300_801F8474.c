#include "bof3/bof3.h"

extern s8 D_80146874;

/* Overlay-local dispatch table of this code span's primary-state handlers: it is
 * the twelve-entry handler run at 0x801FBF48 (the table the exact
 * scenario-state dispatcher dispatchScenarioStateHandlerScenarioScena1300_801F7D14
 * at 0x801F7D14 indexes with the signed state byte D_80146872) read from that
 * run's +0x0C entry, so its own entries are nine: it starts with the inert slot
 * noopHandlerScenarioScena1300_801F84B0 (0x801F84B0), then 0x801F84B8,
 * 0x801F873C, 0x801F8DFC, 0x801F9570, 0x801F9A44, 0x801F9F18, 0x801FA844 and
 * 0x801FB1B4 as its last entry, and it ends immediately before the halfword
 * script data at 0x801FBF78. The embedding run's word 2 (0x801FBF50) holds this
 * file's dispatcher, so entry 2 of that run is the primary-state dispatcher.
 * @source 0x801FBF54 @kind table
 */
extern void (*D_801FBF54[])(void);

/* @source 0x801F8474
 * @behavior Dispatches one frame of this overlay's primary state machine from
 * the signed shared main-RAM state byte D_80146874 (0x80146874): that byte is
 * loaded with a signed byte load, scaled by four onto the overlay-local
 * code-pointer table at 0x801FBF54, and the selected word is called with `jalr`
 * and no arguments. The table's entry 0 is the inert handler
 * noopHandlerScenarioScena1300_801F84B0 (0x801F84B0) and its last entry is
 * 0x801FB1B4, so the signed byte selects which of the overlay's nine primary
 * states runs this frame. It reads no state other than that byte and writes
 * none, takes no arguments and returns nothing of its own; the 0x18-byte frame
 * exists only to keep $ra across the indirect call, the state-byte read is
 * hoisted above the frame setup, and the table load and the `jalr` each keep
 * their delay-slot nop. Address 0x801F8474 is itself entry 2 - the word at
 * 0x801FBF50, payload offset 0x5350 - of the twelve-entry handler run at
 * 0x801FBF48 that the exact scenario-state dispatcher
 * dispatchScenarioStateHandlerScenarioScena1300_801F7D14 (0x801F7D14, word 0 of
 * that run) indexes with the signed state byte D_80146872, so no in-image jal
 * reaches this address. Its 60 bytes are instruction-for-instruction identical
 * to that dispatcher's, differing only in the two symbol immediates (0x80146874
 * for 0x80146872 and 0x801FBF54 for 0x801FBF48), which places it in the same
 * dispatcher family as the exact sibling
 * dispatchPrimaryStateHandlerScenarioScena0700_801FAA2C (0x801FAA2C, the
 * D_80146874 dispatcher of emi/scenario/scena07/00, whose own table 0x801FDE1C
 * is likewise its embedding run read from +0x0C).
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPrimaryStateHandlerScenarioScena1300_801F8474(void) {
  D_801FBF54[D_80146874]();
}
