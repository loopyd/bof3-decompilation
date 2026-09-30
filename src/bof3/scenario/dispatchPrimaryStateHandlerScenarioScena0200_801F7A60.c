#include "bof3/bof3.h"

extern s8 D_80146874;
extern u32 D_801FE294[];

/* @source 0x801F7A60
 * @behavior Dispatches this overlay's per-frame handler named by the signed
 * shared primary-state byte D_80146874 (0x80146874): it loads that byte with a
 * signed byte load, scales it by four onto the overlay's own code-pointer table
 * at 0x801FE294, loads the selected word and calls it with `jalr` and no
 * arguments, returning nothing of its own. It reads no other memory and writes
 * none; the 0x18-byte frame exists only to keep $ra across the indirect call and
 * the state-byte read is hoisted above the frame setup, and both the table load
 * and the `jalr` keep their delay-slot nops. Its 15 instructions are the same
 * shape as the exact sibling dispatchProgressHandlerScenarioScena0200_801F719C
 * (0x801F719C, this overlay), which indexes the adjacent base 0x801FE288 with
 * the signed chain byte D_80146872, and as
 * dispatchProgressHandlerScenarioScena0100_801F8AA0 (0x801F8AA0,
 * emi/scenario/scena01/00), which indexes its own base 0x801FE2A8 with this same
 * D_80146874; only the `lb` byte address and the `lw` table constant differ, so
 * the two families are the same dispatcher over different index bytes and
 * tables. The table is in-image code-pointer data: its entry 0x00 reads
 * 0x801F7A9C and its entry 0x17 reads 0x801FB300, so the 0x17 that
 * requestPrimaryState17AndClearSubstateScenarioScena0200_801FD574 stores into
 * D_80146874 selects that entry. This address is itself the word at 0x801FE290,
 * the third word of the run that begins at 0x801FE288, so the chain dispatcher
 * reaches it when the signed chain byte D_80146872 reads 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPrimaryStateHandlerScenarioScena0200_801F7A60(void) {
  ((void (*)(void))D_801FE294[(s8)D_80146874])();
}
