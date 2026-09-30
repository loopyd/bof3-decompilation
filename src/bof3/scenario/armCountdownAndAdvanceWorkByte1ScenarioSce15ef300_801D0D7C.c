#include "bof3/bof3.h"

/* @source 0x801D0D7C
 * @behavior Entry 0 of the overlay's work-state handler table at 0x801D27CC
 * (D_801D27CC), which the dispatcher func_801D0D38 selects with the unsigned
 * work byte at offset +0x1 of the scratchpad work object published at pointer
 * slot 0x1F800044 and calls with no arguments: it stores 8 into that object's
 * byte at +0x9 and then advances the same work byte at +0x1 to 1, so the next
 * dispatch selects entry 1 (0x801D0DAC). It takes no arguments, returns
 * nothing and reads no other state; each half re-reads the scratchpad pointer
 * cell because the byte store through the loaded pointer may alias that cell.
 * The 0x30-byte instruction stream is byte-identical apart from the immediate
 * (0xF there, 8 here) to the exact lift
 * armCountdownAndAdvanceWorkByte1ScenarioScena0300_801F7AE0 (0x801F7AE0 in
 * emi/scenario/scena03/00), whose documentation reads the +0x9 byte as the
 * work object's countdown; the source shape below is that sibling's.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdownAndAdvanceWorkByte1ScenarioSce15ef300_801D0D7C(void) {
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[9] = 8;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1]++;
}
