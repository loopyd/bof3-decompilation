#include "bof3/bof3.h"

/* @source 0x801D1420
 * @behavior Entry 0 of the overlay work-state handler table at 0x801D2800
 * (D_801D2800), which the dispatcher
 * dispatchWorkByte1HandlerScenarioSce15ef300_801D13DC selects with the unsigned
 * work byte at offset +0x1 of the scratchpad work object published at pointer
 * slot 0x1F800044 and calls with no arguments: it clears that object byte at
 * +0x9, writes 0x10 to its byte at +0x6, and then increments the work byte at
 * +0x1 to 1, so the next dispatch selects entry 1 (0x801D1460). The +0x9 byte
 * is the work countdown that the exact sibling
 * armCountdownAndAdvanceWorkByte1ScenarioSce15ef300_801D0D7C documents in this
 * overlay, and table entry 1 raises it by 0x10 until it wraps through zero;
 * nothing in this payload reads byte +0x6, so that role stays address-anchored.
 * Each store re-reads the scratchpad pointer cell because a byte store through
 * the loaded pointer may alias that cell, which is why the original emits three
 * separate base loads.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkCountdownSetWorkByte6AndAdvanceWorkByte1ScenarioSce15ef300_801D1420(void) {
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[9] = 0;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[6] = 0x10;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1]++;
}
