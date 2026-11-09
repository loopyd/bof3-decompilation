#include "bof3/bof3.h"

/* One-row action table this payload carries at 0x801F615C: the two key bytes
 * 0x4A/0x37, the category nibble 1 and the sub-state 0x14. The original
 * reaches each column through its own absolute symbol with a shared byte
 * offset register, so the columns keep separate array declarations instead of
 * a row view. */
extern u8 D_801F615C[]; /* @source 0x801F615C @kind table */
extern u8 D_801F615D[]; /* @source 0x801F615D @kind table */
extern u8 D_801F615E[]; /* @source 0x801F615E @kind table */
extern u8 D_801F615F[]; /* @source 0x801F615F @kind table */

/* Leading byte of the main-RAM game-mode record, the category the matched
 * row's third column must agree with. Read through a pointer so its address is
 * taken once while the byte stays loaded inside the walk. */
extern u8 D_80145E98; /* @source 0x80145E98 @kind data */

/* Shared state-entry selector: dispatchIndexedStateHandler indexes the
 * D_801C84BC handler table with this byte, and the world overlays publish their
 * next state entry here. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* Shared sub-state byte published together with the state entry. */
extern u8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared front-end startup helper; takes no arguments and answers nothing. */
void func_8015C088(void);

/* @source 0x801F41A0
 * @behavior Matches the two zero-extended word arguments and the low nibble of
 *   the row's third column against the leading byte of the main-RAM game-mode
 *   record 0x80145E98 over the one-row action table this payload carries at
 *   0x801F615C (key bytes 0x4A/0x37, category nibble 1, sub-state 0x14). The
 *   walk advances a byte offset by 0x04 per row and stops at the accepted row;
 *   a run that walks past the single row answers 0 without publishing anything.
 *   On acceptance it runs the shared front-end startup helper func_8015C088,
 *   publishes primary state 0x1F in D_801448EB with the matched row's fourth
 *   byte as the sub-state in D_801448EC, and answers 1. The 0x18-byte frame
 *   keeps $s0 - both the row counter and the index the sub-state column is read
 *   with - and $ra at 0x14. No word and no jal of this payload holds
 *   0x801F41A0, so this handler is reached from outside the payload.
 *
 * Name evidence: 0x801448EB is the state-entry selector that etc/game/00
 *   dispatches its D_801C84BC handler table through and that the world overlays
 *   publish their next state entry into, with 0x801448EC the sub-state of the
 *   same pair; this overlay enters with the tuple 0x1F/0xA from initArea14813State
 *   and its neighbouring entry gate func_801F4108 publishes 0x1F on its success
 *   path, so 0x1F names the requested state family here and D_801448EC the
 *   matched row's sub-state. The arguments carry no evidenced role beyond table
 *   keys, so the name keeps them structural.
 *
 * Shape: the body copies the statement shape of the exact sibling
 *   setTriggerRecordFlagAndQueueCue203OnMatchedArgumentsWorld03Area11813_801F502C
 *   (same-target-payload family: `i = 0`, in-place masks, game-mode address
 *   taken once, `off = 0`, then a `for` over `i <= 0` with a single `&&` guard
 *   and a `break`, followed by `if (i == 1) return 0;`), whose bottom test is the
 *   lone `blez` with the 0x04 offset step in its delay slot, and of
 *   requestPrimaryState14WithMatchedRowSubstateWhenFlag9SetWorld03Area14514_801F4504,
 *   which publishes the same state-entry pair with the matched row's fourth byte
 *   as the sub-state. This span's own literals come from its bytes: one row bound
 *   1 (the range has no seven-row walk), public state 0x1F in place of 0x14, and
 *   no flag probe between the walk and the publish.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestPrimaryState1FWithMatchedRowSubstateWorld03Area14813_801F41A0(
    s32 arg0, s32 arg1) {
  s32 i;
  s32 off;
  u8* mode;

  i = 0;
  arg0 &= 0xFF;
  arg1 &= 0xFF;
  mode = &D_80145E98;
  off = 0;
  for (; i <= 0; i++, off += 4) {
    if (arg0 == D_801F615C[off] && arg1 == D_801F615D[off] &&
        (D_801F615E[off] & 0xF) == *mode) {
      break;
    }
  }
  if (i == 1) {
    return 0;
  }
  func_8015C088();
  D_801448EB = 0x1F;
  D_801448EC = D_801F615F[i * 4];
  return 1;
}
