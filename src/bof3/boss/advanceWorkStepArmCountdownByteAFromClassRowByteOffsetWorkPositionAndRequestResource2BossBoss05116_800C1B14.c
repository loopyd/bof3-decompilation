#include "bof3/bof3.h"

/* Scratch work record pointer cell: the battle code publishes the scratch work
 * record here and every access in this payload reaches it through this cell.
 * The original re-reads the cell before each store group, because a store
 * through the loaded pointer may alias the cell, and it materializes the cell
 * address once per read (lui $vN,0x1F80 / lw $vN,68($vN)); the named cell is the
 * spelling that reproduces that, as in the exact sibling lift at 0x800C1CE4. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer the main program publishes and its battle
 * overlays read back: the bmagic lifts read this cell as the shared battle work
 * record pointer and this overlay publishes it into the scratchpad cell before
 * the update it performs. Splat binds the address through this target's
 * generated undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0, so no
 * target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Class byte of the 0x118-stride local panel record run: the leading record's
 * byte selects the 0x88-byte class row read below. Same declaration and same
 * spelling as the exact boss052 lift at 0x800C1DF8 and as the
 * battle15/battle03 internal headers, which bind the byte for the same row
 * family. */
extern u8 D_801EB710[]; /* @source 0x801EB710 @kind unknown */

/* Shared main-executable resource request helper: the sibling boss lifts of
 * this family declare the same prototype and pass a resource id. */
void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */

/* @source 0x800C1B14
 * @behavior Entry 0 of this overlay's step table at 0x800C1804, the four-word
 *           run (0x800C1B14, 0x800C1BF0, 0x800C1CE4, 0x800C1D7C) that
 *           dispatchStateByte1Table1804BossBoss05116_800C1AA8 copies into its
 *           frame and indexes with the work record byte 1, so no jal names this
 *           entry. It loads the scratch work record pointer published at the
 *           scratchpad cell 0x1F800044, advances that record's byte 1 by one,
 *           which moves the next byte-1 dispatch to word 1 (0x800C1BF0), and
 *           copies the class-row byte at 0x800E40CE selected by the 0x88-byte
 *           class stride off the leading class byte D_801EB710 into work byte
 *           0x09; the entry it switches to tests that byte against 0xFF and 0
 *           and clears it, so this store selects the follow-up work. It then
 *           stores 0x2D into work byte 0x0A, the countdown the next entry
 *           decrements before it advances again, saves the cell pointer it
 *           loaded, publishes the shared battle work pointer D_801EB4E0 in the
 *           cell instead, raises bit 0x80 of that record's byte 0x24, stores 2
 *           into its byte 0x29, offsets its position halfwords 0x2E and 0x30 by
 *           +0x30 and -0x10, requests resource 2 through the shared helper
 *           func_8014D6B8 and restores the saved cell pointer afterwards. The
 *           pointer cell read precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x14 and $s0 at 0x10 across that call, and the cell is
 *           re-read before each store group because a store through the loaded
 *           pointer may alias it. Roles of work bytes 0x24, 0x29 and the
 *           position halfwords beyond the same-field access in the bmagic lifts
 *           are not established here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkStepArmCountdownByteAFromClassRowByteOffsetWorkPositionAndRequestResource2BossBoss05116_800C1B14(
    void) {
  u8 *work;
  u8 *slot;
  u8 *countdown;
  u8 *record;
  u8 *shared;
  u8 *previous;
  u16 x;
  u16 y;

  work = D_1F800044;
  work[1]++;
  slot = D_1F800044;
  slot[9] = PSX_REF(u8, 0x800E40CEu + D_801EB710[0] * 0x88u);
  countdown = D_1F800044;
  countdown[0xA] = 0x2D;
  shared = D_801EB4E0;
  previous = D_1F800044;
  D_1F800044 = shared;
  shared[0x24] |= 0x80;
  record = D_1F800044;
  record[0x29] = 2;
  x = *(u16 *)(record + 0x2E);
  y = *(u16 *)(record + 0x30);
  *(u16 *)(record + 0x2E) = x + 0x30;
  *(u16 *)(record + 0x30) = y - 0x10;
  func_8014D6B8(2);
  D_1F800044 = previous;
}
