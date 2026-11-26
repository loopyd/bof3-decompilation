#include "bof3/bof3.h"

/* @source 0x801F2CC4
 * @behavior Entry 0 of the four-word work-byte-1 handler table at 0x801F3C54,
 * the entry the neighbouring dispatchStateByte1Table3
 * dispatcher selects with byte 0x01 of the scratch work record published at the
 * scratchpad pointer cell 0x1F800044. It re-reads that cell and zeroes the
 * record's three colour-channel bytes at 0x5F, 0x5E and 0x5D in that store
 * order, the channels the fade tile drawn by
 * renderFullscreenColorTile shades with; it
 * re-reads the cell and seeds the record's countdown byte at 0x09 with 0x3C
 * (60), the countdown that the next entry, 0x801F2D00, decrements while it
 * ramps those same three channels by one per frame; and it re-reads the cell
 * once more to move the record's dispatch byte at 0x01 to 1, so the next frame
 * runs that entry. It reads and writes no other state, takes no arguments and
 * returns nothing. Each of the three halves re-reads the pointer cell because
 * the byte stores through the loaded pointer may alias it, and the final store
 * sits in the jr delay slot. The 0x3C-byte span repeats the exact sibling lift
 * clearWorkTintArmCountdownAndAdvanceWorkByte1ScenarioScena1300_801F6EE8
 * (0x801F6EE8, scena13/00), and one cmp of the two spans showed that only word
 * 8, this body's own 0x3C countdown immediate, differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateColorTripletArm(
    void) {
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[0x5D] =
      PSX_REF(u8*, SPAD_BASE + slot_offset)[0x5E] =
          PSX_REF(u8*, SPAD_BASE + slot_offset)[0x5F] = 0;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[9] = 0x3C;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1] = 1;
}
