#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound by this target's map row
 * g_battle_work = 0x1F800044. The original re-materialises the cell before each
 * access group because a store through the pointer it supplied may alias the
 * cell, so the body reads the named cell each time instead of holding one copy.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Frame counter of the main executable, outside this overlay image; bit 0 is
 * the frame parity this body tests. The shared symbol map binds the address, so
 * no target-map row is added. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* @source 0x801EF44C
 * @behavior Advances the scratchpad work object one frame. The pointer cell at
 *           0x1F800044 is read once, then work bytes 0xB and 9 are read through
 *           that pointer, one plus work byte 9 shifted right by six is added to
 *           byte 0xB and the sum stored at byte width, so byte 0xB gains one
 *           plus a sixth of byte 9. When bit 0 of the frame counter at
 *           0x80143E6C is set the cell is re-read and work byte 0xA increments.
 *           The cell is re-read again, two is added to work byte 9 at byte
 *           width, and only when the stored byte, truncated to eight bits,
 *           equals 0xC0 does the body re-read the cell once more and increment
 *           work byte 2. The function address is published at index five of the
 *           overlay-local handler run based at 0x801F04B8, so no direct jal
 *           reaches it. It takes no arguments, returns nothing, keeps no frame
 *           and writes no state outside the work object. Roles of work bytes 2,
 *           0xA, 0xB and 9 stay address-anchored.
 *
 *           The 0xA8-byte span is byte-identical to the exact lift
 *           src/bof3/bmagic/advanceWorkByteBIncrementByteAOnGlobalBit0AndByte9ByTwoThenAdvanceByte2At192BmagicMagic13703_801EF174.c
 *           of emi/bmagic/magic137/03 0x801EF174; a payload comparison of the
 *           two spans reported no differing byte, so that body was cloned here
 *           with its own symbols and this target's cell bindings. The nearest
 *           lifted sibling inside this target is 0x801EF4F4, the same body with
 *           inverted operations: a payload comparison of the two spans reported
 *           three differing words, addiu -1 instead of addiu 1 on work byte
 *           0xA, addiu -2 instead of addiu 2 on work byte 9, and the immediate
 *           0x60 instead of 0xC0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByteBIncrementByteAOnGlobalBit0AndByte9ByTwoThenAdvanceByte2At192BmagicMagic06703_801EF44C(void) {
  u8 *work;
  u32 next;

  work = g_battle_work;
  next = work[0xB] + 1;
  work[0xB] = next + (work[9] >> 6);
  if (D_80143E6C & 1) {
    g_battle_work[0xA]++;
  }
  if ((g_battle_work[9] += 2) == 0xC0) {
    g_battle_work[2]++;
  }
}
