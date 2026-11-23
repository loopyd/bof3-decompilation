#include "bof3/bof3.h"

/* @source 0x801F2D3C
 * @behavior Entry 3 of this overlay's nine-word work-state handler run at
 *           0x801F36CC, the table the exact dispatcher
 *           callStateTable36World03 indexes with
 *           the unsigned work byte at offset +0x1 of the work object published
 *           at the scratchpad pointer cell 0x1F800044 and calls with no
 *           arguments. It stores 0x96 into that object's byte at +0x9 and then
 *           advances the same work byte at +0x1 by one, so the next frame
 *           dispatches entry 4 (0x801F2D6C), which decrements the byte at +0x9
 *           once per frame and calls 0x801F3268 with the mode word 2 whenever
 *           that byte is a multiple of 0x0F; the armed 0x96 is ten of those
 *           steps. Each half re-reads the scratchpad pointer cell, because the
 *           byte store through the loaded pointer may alias the cell itself.
 *           The function takes no arguments, returns nothing, calls nothing,
 *           reads no other state and needs no frame, so its 0x30 bytes carry
 *           no prologue. The instruction stream is byte-identical apart from
 *           the armed immediate to the exact lifts
 *           armCountdownAndAdvanceWorkByte1ScenarioScena0300_801F7AE0
 *           (0x801F7AE0 in emi/scenario/scena03/00, arming 0xF) and
 *           armCountdownAndAdvanceWorkByte1ScenarioSce15ef300_801D0D7C
 *           (0x801D0D7C in emi/scenario/sce15ef3/00, arming 8), so the
 *           statement order and operand forms below are that family's, with
 *           this span's own armed value and record offsets. The roles of the
 *           bytes at +0x1 and +0x9 stay described by offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdownAdvanceState(
    void) {
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[9] = 0x96;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1]++;
}
