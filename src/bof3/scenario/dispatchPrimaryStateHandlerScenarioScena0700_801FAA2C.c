#include "bof3/bof3.h"

extern s8 D_80146874;

/* Overlay-local dispatch table of this code span's primary-state handlers: it
 * is the twelve-entry handler run at 0x801FDE10 (the table the exact
 * scenario-state dispatcher dispatchScenarioStateHandlerScenarioScena0700_801FA65C
 * at 0x801FA65C indexes with the signed state byte D_80146872) read from that
 * run's +0x0C entry, so its own entries are nine: it starts with the inert slot
 * noopHandlerScenarioScena0700_801FAA68 (0x801FAA68), then 0x801FAA70,
 * 0x801FABD8, 0x801FB6DC, 0x801FB914, 0x801FC1C4, 0x801FC2D0 and 0x801FD0DC, and
 * ends at 0x801FD35C immediately before the halfword script data at 0x801FDE40.
 * @source 0x801FDE1C @kind table
 */
extern void (*D_801FDE1C[])(void);

/* @source 0x801FAA2C
 * @behavior Dispatches one frame of this overlay's primary state machine from
 * the signed shared main-RAM state byte D_80146874: that byte is scaled by four
 * and selects one of the nine entries of the overlay-local handler table at
 * 0x801FDE1C, whose entry 0 is the inert handler
 * noopHandlerScenarioScena0700_801FAA68 (0x801FAA68) and which runs through
 * 0x801FD35C, and the selected entry is invoked. It reads no state other than
 * that byte and writes none: it takes no arguments and returns nothing, and the
 * 0x18-byte frame exists only to keep $ra across the indirect call, with the
 * state-byte read hoisted above the frame setup. Address 0x801FAA2C is itself
 * entry 2 of the twelve-entry handler run at 0x801FDE10 that the exact
 * scenario-state dispatcher dispatchScenarioStateHandlerScenarioScena0700_801FA65C
 * (0x801FA65C) indexes with the signed state byte D_80146872; its 60 bytes are
 * instruction-for-instruction identical to that dispatcher's, differing only in
 * the two symbol immediates (D_80146874 for D_80146872 and D_801FDE1C for
 * D_801FDE10). The scena00 overlay pairs the same two tables at 0x801FCA10 and
 * 0x801FCA1C, where the D_80146874-indexed dispatcher func_801F9B98 is the
 * primary state machine and D_801FCA1C is its table read from the same +0x0C
 * offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPrimaryStateHandlerScenarioScena0700_801FAA2C(void) {
  D_801FDE1C[D_80146874]();
}
