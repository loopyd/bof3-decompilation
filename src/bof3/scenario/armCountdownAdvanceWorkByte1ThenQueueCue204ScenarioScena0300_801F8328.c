#include "bof3/bof3.h"

void func_8015DF18(u16 arg0);

/* @source 0x801F8328
 * @behavior Entry 0 of the overlay's work-object handler table at 0x801FCF94
 * (the three words 0x801F8328 / 0x801F8370 / 0x801F842C), which the frame
 * dispatcher dispatchWorkByte1HandlerScenarioScena0300_801F82E4 selects with
 * unsigned byte 0x1 of the work object published at scratchpad pointer cell
 * 0x1F800044 and calls with no arguments: it arms that object's countdown byte
 * at +0x9 with 0x60 (96), advances the work byte at +0x1 by one so the next
 * frame dispatches entry 1 (0x801F8370, which spawns records while the armed
 * countdown byte still has bit 2 set, decrements the byte and resolves its own
 * tail by advancing the same work byte again) and queues the main-exe sound cue
 * 0x204 through func_8015DF18. It takes no arguments, returns nothing of its
 * own and the 0x18-byte frame exists only to hold $ra across the cue call; the
 * cell is re-read for the second store group because the byte store through the
 * loaded pointer may alias that cell.
 * The slot-offset spelling and statement order are the ones the sibling lift of
 * the same instruction stream without the cue call
 * (src/bof3/scenario/armCountdownAndAdvanceWorkByte1ScenarioScena0300_801F7AE0.c,
 * identical here except for the countdown immediate 0xF/0x60 and this target's
 * extra cue call) reproduces.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdownAdvanceWorkByte1ThenQueueCue204ScenarioScena0300_801F8328(void) {
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[9] = 0x60;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1]++;
  func_8015DF18(0x204);
}
