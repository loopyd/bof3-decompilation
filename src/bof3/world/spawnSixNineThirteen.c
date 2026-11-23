#include "bof3/bof3.h"

void claimFreeRingDirection(
    s32 slot);

/* @source 0x801F2C80
 * @behavior Entry 1 of this overlay's nine-word work-state handler run at
 *           0x801F36CC, the table the exact dispatcher
 *           callStateTable36World03 indexes with
 *           the unsigned work byte at offset +0x1 of the work object published
 *           at the scratchpad pointer cell 0x1F800044; entry 0 at 0x801F2C48
 *           advances that byte to 1, so this body runs on the second frame of
 *           the run. It claims and seeds three ring records through the
 *           overlay's spawn at 0x801F31B0 with the ring slot indices 0x6, 0x9
 *           and 0xD: that spawn scans the 0x28-byte-stride record table at
 *           0x800E4800 for a free entry and fills it from the 16-entry ring
 *           position and direction tables at 0x801F364C and 0x801F368C, so the
 *           three seeded records are fixed points of the 16-step ring, laid
 *           down in slot order rather than the shuffled order entry 6 uses. It
 *           then stores 0x3C into the work object's byte at +0x9 and advances
 *           the same work byte at +0x1 by one, so the next frame dispatches
 *           entry 2 at 0x801F2CD8, which renders the record sweep, decrements
 *           the byte at +0x9 once per frame and only advances the work byte to
 *           entry 3 (0x801F2D3C) when that byte reaches zero: the 0x3C armed
 *           here is a 60-frame stage before entry 3 arms the longer 0x96
 *           segment. The byte store and the work-byte advance each re-read the
 *           scratchpad pointer cell, because the store through the loaded
 *           pointer may alias the cell itself. The function takes no
 *           arguments, returns nothing and reads no other state; its 0x18-byte
 *           frame holds the return address across the three calls.
 *
 * Shape evidence: the arm-then-advance tail is byte-identical, apart from the
 *           call head and frame, to the exact lift
 *           incrementWorkByte1SetCountdownScenarioScena0800_801F98B0
 *           (0x801F98B0 in emi/scenario/scena08/00), so the slot-offset local
 *           and the two re-evaluated PSX_REF accesses below are that sibling's,
 *           with this span's own countdown value. The leading frame plus a
 *           literal-argument call whose argument is set in the jal delay slot
 *           follows the exact lifts of this repository (for example VSync(3)
 *           at 0x80161F70 in initEmiLoader); the three call targets and the
 *           constants 0x6, 0x9, 0xD and 0x3C come from this span's own bytes.
 *           No extracted payload holds this 0x58-byte body a second time, so no
 *           byte-identical sibling seeded it and the clone lever was declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void spawnSixNineThirteen(
    void) {
  u32 slot_offset;

  claimFreeRingDirection(
      0x6);
  claimFreeRingDirection(
      0x9);
  claimFreeRingDirection(
      0xD);
  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[9] = 0x3C;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1]++;
}
