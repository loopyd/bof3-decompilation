#include "bof3/bof3.h"

/* @source 0x801F6EE8
 * @behavior Entry 0 of this overlay's work-object handler table at 0x801FBEF0,
 * the entry the overlay frame dispatcher func_801F6EA4 selects with byte 0x01
 * of the scratchpad work object published at pointer slot 0x1F800044: it
 * re-reads that pointer cell and zeroes the work object's three colour-channel
 * bytes at 0x5F, 0x5E and 0x5D in that store order (clearing the tint triple
 * that this overlay's other work handlers ramp by 7), re-reads the cell and arms
 * the object's countdown byte at 0x09 with 0xF (15), and re-reads the cell once
 * more and moves the object's dispatch byte at 0x01 to 1, so the next frame runs
 * entry 1 (0x801F6F24), which decrements that countdown byte and ramps those
 * same three colour bytes. It reads no state other than the scratchpad pointer
 * cell, takes no arguments and returns nothing; each of the three halves
 * re-reads the cell because the byte store through the loaded pointer may alias
 * it, and the last store sits in the jr delay slot. The address is word 0 of the
 * twelve-word table at 0x801FBEF0 (0x801FBEF0 through 0x801FBF1C), and its 60
 * bytes repeat the shape of the exact sibling
 * armCountdownAndAdvanceWorkByte1ScenarioScena0300_801F7AE0 (0x801F7AE0,
 * scena03/00), which is entry 0 of that overlay's own work-object handler table
 * and arms the same countdown byte with the same 0xF before advancing the same
 * dispatch byte, and of the exact lift func_801F7264 (0x801F7264, scena00/00),
 * which is the byte-identical instruction stream apart from its 0xF0 countdown
 * immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTintArmCountdownAndAdvanceWorkByte1ScenarioScena1300_801F6EE8(void) {
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[0x5D] =
      PSX_REF(u8*, SPAD_BASE + slot_offset)[0x5E] =
          PSX_REF(u8*, SPAD_BASE + slot_offset)[0x5F] = 0;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[9] = 0xF;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1] = 1;
}
