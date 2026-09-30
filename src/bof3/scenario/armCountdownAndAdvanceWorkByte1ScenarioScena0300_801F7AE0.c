#include "bof3/bof3.h"

/* @source 0x801F7AE0
 * @behavior Entry 0 of the overlay's work-object handler table at 0x801FCF70
 * (D_801FCF70), which the frame dispatcher func_801F7A9C selects with byte 0x1
 * of the work object published at scratchpad pointer slot 0x1F800044 and calls
 * with no arguments: it arms that work object's countdown byte at +0x9 with 0xF
 * (15) and then advances the work byte at +0x1 to 1, so the next frame
 * dispatches entry 1 (0x801F7B10), which decrements the same countdown byte and
 * re-arms it to 8 whenever it reaches zero. Takes no arguments and returns
 * nothing; each half re-reads the scratchpad pointer cell because the byte
 * store through the loaded pointer may alias that cell.
 * The slot-offset spelling is the one the sibling lift of the same 0x30-byte
 * instruction stream (src/bof3/scenario/incrementWorkByte1SetCountdownScenarioScena0800_801F98B0.c,
 * identical here except for the countdown immediate 0xF/0x3C) uses, and it is
 * what reproduces the shipped $v1 pointer / $v0 value register pattern.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdownAndAdvanceWorkByte1ScenarioScena0300_801F7AE0(void) {
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[9] = 0xF;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1]++;
}
