#include "bof3/bof3.h"

/* @source 0x801D10E8
 * @behavior Entry 0 of this overlay's 3-entry handler table at 0x801D1B48
 * (0x801D10E8, 0x801D1118, 0x801D11A4), which the dispatcher func_801D10A4
 * selects with the unsigned work byte at offset +0x1 of the scratchpad work
 * object published at pointer slot 0x1F800044 and calls with no arguments: it
 * arms that object's countdown byte at +0x9 with 0x10 and then advances the
 * same work byte at +0x1 to 1, so the next frame dispatches entry 1
 * (0x801D1118), which decrements that countdown and re-arms it. Each half
 * re-reads the scratchpad pointer cell because the byte store through the
 * loaded pointer may alias that cell. It takes no arguments and returns
 * nothing. The 0x30-byte instruction stream is byte-identical apart from the
 * countdown immediate to armCountdownAndAdvanceWorkByte1ScenarioSce15ef300_801D0D7C
 * (0x801D0D7C in emi/scenario/sce15ef3/00, immediate 8).
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdownAndAdvanceWorkByte1ScenarioSce15ef200_801D10E8(void) {
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[9] = 0x10;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1]++;
}
