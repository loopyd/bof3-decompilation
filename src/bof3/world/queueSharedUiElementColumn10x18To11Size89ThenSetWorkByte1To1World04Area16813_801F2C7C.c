#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. This body
 * addresses the record through the named cell at its single store, because the
 * original loads the cell once and keeps no copy of the pointer.
 * @source 0x1F800044 @kind data
 */
extern u8* area16813WorkCursor_1F800044;

/* Six-entry byte row table of this payload's own textbin tail at 0x801F4B2C:
 * entry k is the second argument the body hands to the shared world-marker
 * queue, and the bytes read 0x18, 0x17, 0x15, 0x14, 0x12, 0x11. The tail pads
 * the run to eight bytes and starts the state-byte-1 handler table at
 * 0x801F4B38, whose word 0 is this body.
 * @source 0x801F4B30 @kind data
 */
extern u8 D_801F4B30[];

/* Shared world-marker queue of the shared main executable; the sibling handlers
 * of this payload hand it a marker coordinate pair and a size.
 * @source 0x801A4BC0
 */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2C7C
 * @behavior Word 0 of the state-byte-1 handler run based at 0x801F4B38 of this
 *           payload, whose dispatcher is 0x801F2C38: the body address appears
 *           once in the payload bytes, at word 0x801F4B38, and no `jal` in the
 *           payload names it, so it takes no arguments and answers no value.
 *           The body walks the six entry byte row table 0x801F4B30 with a
 *           narrow counter and queues one shared world marker through
 *           0x801A4BC0 per entry, all at the coordinate 0x10 of the marker grid
 *           and at the entry's own second coordinate, with the literal size
 *           0x89. After the run it reads the work record through the scratchpad
 *           pointer cell 0x1F800044 and stores 1 into record byte +0x01, the
 *           byte the 0x801F2C38 dispatcher indexes, so the next dispatch of
 *           that run takes word 1, 0x801F2CE0. The 0x18-byte frame only keeps
 *           $ra and $s0, and the cell is read once, after the loop.
 *
 * Name evidence: 0x801F4B38 is the state-byte-1 handler table this payload's
 * exact source dispatchStateByte1Table4b38World04Area16813_801F2C38 installs,
 * and the body's +0x01 store is the state byte that dispatcher reads, so the
 * storage moves the run on to word 1 rather than completing an overlay step;
 * the record byte therefore stays named after its dispatch role. 0x801A4BC0 is
 * the shared world-marker queue the exact world04 sibling sources of this tree
 * name that way (queueSharedUiElement9x3Size50World04Area17213_801F4D58 and the
 * block spellings beside it), and they read its three arguments as a marker
 * coordinate pair followed by a size, which is the vocabulary this name uses
 * for the three call arguments. The exact sibling source of this payload,
 * stateDistanceUiElement,
 * reads the same textbin tail's byte columns as one row table and calls the
 * same queue, so 0x801F4B30 keeps the row-table wording and stays
 * address-anchored, as does every record field this body touches.
 *
 * Shape evidence: the in-target `asm` sibling 0x801F2CE0, word 1 of the same
 * table, repeats this loop idiom byte for byte in structure: a `u8` counter
 * initialised to zero, the counter masked with `andi 0xFF` before the loop
 * head, the increment rotated to the loop head, and the bound written as
 * `andi`/`sltiu`/`bnez` at the bottom with the `andi` repeated in the branch
 * delay slot. Its own `sb` of 2 into record byte +0x01 shows the same
 * dispatch-byte store this body writes with 1. The span's own bytes carry no
 * twin anywhere in `out/binaries` (the 0x64-byte body occurs once, at this
 * target's offset 0x7C), so no clone applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSharedUiElementColumn10x18To11Size89ThenSetWorkByte1To1World04Area16813_801F2C7C(
    void) {
  u8 i;

  for (i = 0; i < 6; i++) {
    func_801A4BC0(0x10, D_801F4B30[i], 0x89);
  }
  area16813WorkCursor_1F800044[1] = 1;
}
