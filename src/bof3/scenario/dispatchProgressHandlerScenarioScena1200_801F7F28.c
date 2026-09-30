#include "bof3/bof3.h"

extern s8 D_80146872;
extern u32 D_801FD1B8[];

/* @source 0x801F7F28
 * @behavior Dispatches one frame of this overlay's scenario state chain from
 * the signed shared scenario state byte D_80146872 (0x80146872): the byte is
 * scaled by four and selects an entry of the overlay-local handler table at
 * 0x801FD1B8 (44 code-pointer words, 0x801F7F64 through 0x801FC7D4), and the
 * selected entry is then invoked with `jalr` and no arguments. Entry 0 of that
 * table (0x801F7F64) writes 1 to D_80146872 itself and its last entry
 * (0x801FC7D4) requests primary state 1 with sub-state 0x0A, so the byte walks
 * the chain forward one entry at a time. The address is entry 11 (the word at
 * 0x801FD1A4) of the overlay's per-frame handler table at 0x801FD178, which
 * func_801F7174 indexes with the work-object state byte at offset 0x01, so this
 * is the frame state that hands the frame to the state chain. It reads no state
 * other than that byte and writes none; the 0x18-byte frame exists only to keep
 * $ra across the indirect call. The shape is the one the D_80146872 progress
 * dispatchers of the other EMI scenario overlays use
 * (dispatchProgressHandlerScenarioScena0400_801F8124 0x801F8124), hence the
 * progress-handler dispatch name; the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressHandlerScenarioScena1200_801F7F28(void) {
  ((void (*)(void))D_801FD1B8[(s8)D_80146872])();
}
