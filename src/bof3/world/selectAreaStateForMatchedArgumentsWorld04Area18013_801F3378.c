#include "bof3/bof3.h"

/* Leading byte of the main-RAM game-mode record, read here with a single `lbu`.
 * @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;
/* Signed leading byte of the shared scenario-state record, taken with `lb` and
 * tested against 8.
 * @source 0x80146870 @kind unknown */
extern s8 D_80146870;
/* Shared front-end state-entry selector that the world overlays publish their
 * next state entry into and that etc/game/00's indexed dispatcher reads as a
 * handler-table index.
 * @source 0x801448EB @kind unknown */
extern u8 D_801448EB;

/* @source 0x801F3378
 * @behavior Overlay request gate that selects the shared front-end state entry
 *   0x801448EB: it returns 0 unless both zero-extended halfword arguments read
 *   0x3F and 0x0E (each truncated with `andi 0xFFFF`, so both are u16) and the
 *   leading byte of the main-RAM game-mode record 0x80145E98 reads 1. On a
 *   match it stores 6 while the signed scenario byte 0x80146870 is below 8 and
 *   0x1A otherwise, then returns 1. The original body funnels that select into
 *   one `sb`: 6 while the byte is below 8 and 0x1A otherwise, with the 0x1A
 *   `addiu` in the `j` delay slot and the below-8 arm out of line; every failing
 *   guard answers through the shared `jr $ra` with `addu $v0,$zero,$zero` in its
 *   branch delay slot. No calls, no frame and no other stores.
 *
 * Name evidence: 0x801448EB is the state selector that etc/game/00 dispatches
 *   its handler table through (dispatchIndexedStateHandler reads it as an s8
 *   table index) and that the world overlays set as their state entry; 0x80146870
 *   is the leading byte of the etc/game/00 scenario-state record, declared s8 at
 *   its original `lb` width in include/bof3/world/area01613_internal.h and
 *   include/bof3/ui/shop00_internal.h; 0x80145E98 is the leading byte of the
 *   main-RAM game-mode record, read here with `lbu`. The two arguments carry no
 *   evidenced role, so the name keeps them structural.
 *
 * Shape: the 108-byte span is byte-identical to the same function in
 *   emi/world04/area177/13 (one payload `cmp` of span 0x778..0x7E4 matches) and
 *   to areas 176 and 178 through 185, but no byte-identical twin in the tree is
 *   itself lifted and exact, so the body is seeded from this target's own bytes
 *   with the arm-store statement order and declaration style of the exact
 *   world04 gate sources.
 *
 * Residual: measured 74.07% (20/27 instructions, 108->100 bytes), first
 *   difference at instruction 2, the first guard's `bne` target. The three
 *   guards, both cells' loads and the select's own `slti`/branch operands match;
 *   the two missing instructions are the unfilled branch delay `nop` and the `j`
 *   over the below-8 arm. `.jump2` RTL for every measured clean-C shape already
 *   holds that shape (branch to the 6 arm, `j` to the join, arm value in the
 *   `j` delay slot), and `bin/harness lift sweep` shows the difference is made
 *   later by GCC 2.7.2 `reorg.c` `dbr_schedule`: it steals the arm constant out
 *   of the branch target block into the branch's delay slot, redirects the
 *   branch at the join and deletes the `j`. Steering `own_thread_p` or the steal
 *   refusal for that target block was not reachable from any measured clean-C
 *   spelling.
 * @status partial
 * @match 74.07
 * @residual branch delay slot filled from the target arm and the j over it deleted by dbr_schedule, 108->100 bytes
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area18013_801F3378(u16 arg0,
                                                                u16 arg1) {
  if (arg0 != 0x3F) {
    return 0;
  }
  if (arg1 != 0xE) {
    return 0;
  }
  if (D_80145E98 != 1) {
    return 0;
  }
  if (D_80146870 < 8) {
    D_801448EB = 6;
  } else {
    D_801448EB = 0x1A;
  }
  return 1;
}
