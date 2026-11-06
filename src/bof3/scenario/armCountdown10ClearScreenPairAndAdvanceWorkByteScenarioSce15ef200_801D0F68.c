#include "bof3/bof3.h"

/* @source 0x801D0F68
 * @behavior Entry 7 of the overlay's code-pointer run at 0x801D1B1C, which the
 * overlay entry func_801D0C04 selects with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044 and
 * calls with no arguments; the run carries no bound check, so work byte 8
 * reaches the following word at 0x801D1B3C (0x801D0F9C). It arms that object's
 * countdown byte at +0x9 with 0xA, clears the projected screen-pair halfwords
 * at +0x2E and +0x30 that sibling lifts of this work object read as reported x
 * and y, and then advances the same work byte at +0x1 by one, so the next frame
 * dispatches the word after this entry. The scratchpad pointer cell is read
 * twice because the byte store through the loaded pointer may alias that cell.
 * It takes no arguments and returns nothing. The 0x34-byte instruction stream
 * is the shape of the exact lift
 * setScratchWorkByte9ClearHalf2eAdvanceStateWorld04Area17213_801F3034
 * (0x801F3034 in emi/world04/area172/13) with this entry's countdown store and
 * second halfword clear added; the pointer-cell spelling is the one the exact
 * sibling armCountdownAndAdvanceWorkByte1ScenarioSce15ef200_801D10E8 in this
 * target uses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdown10ClearScreenPairAndAdvanceWorkByteScenarioSce15ef200_801D0F68(void) {
  u32 slot_offset;
  u8* work;
  u8* next;

  slot_offset = 0x44u;
  work = PSX_REF(u8*, SPAD_BASE + slot_offset);
  work[9] = 0xA;
  next = PSX_REF(u8*, SPAD_BASE + slot_offset);
  *(u16*)(work + 0x2E) = 0;
  *(u16*)(work + 0x30) = 0;
  next[1]++;
}
