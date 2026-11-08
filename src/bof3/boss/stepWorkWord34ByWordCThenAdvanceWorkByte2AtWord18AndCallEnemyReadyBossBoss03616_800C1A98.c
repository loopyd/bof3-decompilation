#include "bof3/bof3.h"

/* Shared main-program readiness helper: it is enemyReadyOrHelper1 of
 * emi/battle/battle/03, at 0x801E30F8 in the main executable, and this target's
 * generated undefined_funcs_auto.txt already binds the address. This handler is
 * declared void because the dispatcher that steps it discards the result, yet
 * the helper's $v0 is left untouched by the epilogue below, so the value still
 * survives into that caller. */
u8 func_801E30F8(void);

/* @source 0x800C1A98
 * @behavior Work-record step of the overlay's byte-2 handler run at 0x800C206C:
 * it takes the work-record pointer published at the scratchpad pointer cell
 * 0x1F800044 and rewrites that record's word 0x34 as word 0x34 minus word 0xC,
 * the step the record's own cursor takes this frame. The rewritten word is
 * stored unconditionally, in the branch delay slot, and compared against the
 * record word 0x18; when the two are equal it increments the record's unsigned
 * byte 0x2, which advances the byte-2 handler run one entry, and otherwise it
 * leaves that byte alone. The frame is the 0x18-byte non-leaf frame that keeps
 * $ra at 0x10 across the tail call, whose argument register $a1 is already the
 * record pointer and is not restored. It ends by calling the shared readiness
 * helper at 0x801E30F8 and leaves that helper's $v0 in place, so this handler
 * takes no arguments and its own result is the helper's, discarded by the
 * dispatcher.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkWord34ByWordCThenAdvanceWorkByte2AtWord18AndCallEnemyReadyBossBoss03616_800C1A98(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44);
  s32 delta;

  delta = *(s32 *)(work + 0x34) - *(s32 *)(work + 0xC);
  *(s32 *)(work + 0x34) = delta;
  if (delta == *(s32 *)(work + 0x18)) {
    work[2] = (u8)(work[2] + 1);
  }
  func_801E30F8();
}
