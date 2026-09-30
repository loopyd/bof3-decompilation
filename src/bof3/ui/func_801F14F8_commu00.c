#include "bof3/ui/commu00_internal.h"

/* @source 0x801F14F8
 * @behavior Stores the incoming task as the current commu00 task, requests UI
 * mode 0x17, refreshes the active UI, and stores 1 into the byte at 0x801448F2.
 * @status partial
 * @match 87.50
 * @residual Post-call constant store materializes 1 in $v0 where the original uses $v1; first difference +0x0024, 14/16 instructions, 64/64 bytes, frame 0x18 unchanged.
 * Clean-C attempts that all reproduced $v0: early result variable plus return, two named constant locals, and a volatile view of the stored byte.
 * Eight further clean-C spellings measured this campaign, all reproducing $v0 at the same +0x0024 first difference: a result local assigned 0 before the store, after the store, and as a declaration initializer; word- and byte-typed store locals; a byte local initialized at function entry; and the store duplicated ahead of the call (that spelling regresses to 12/16, first difference +0x0008).
 * Mechanism: exact siblings 0x801F1318/0x801F13B8 carry the same trailing shape (post-call constant byte store, then return 0) and emit $v1 because their return-value set is ordered ahead of the constant store when reload assigns it; here the same set stays behind the store, leaving $v0 free. Residual class: reload/allocator register choice fixed by pre-allocation insn order rather than by an untried clean-C type/control-flow/expression spelling. Next evidence: an authorized compiler-profile probe or a scheduler-order route; independent review pending.
 */
s32 func_801F14F8(Commu00TaskSlot *task) {
  currentCommu00Task = task;
  uiMode = 0x17;
  func_8015C088();
  D_801448F2 = 1;
  return 0;
}
