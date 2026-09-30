#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6D28
 * @behavior Returns when the 0x801462E8 halfword carries bit 0x400 and the
 * current work record byte +5 already equals the global byte 0x80146374.
 * Otherwise it reads the status halfword of the record named by that byte --
 * local work +0x80 below three, enemy work +0x82 (index - 3) at or above three
 * -- returns when func_801DB524 reports that record unavailable, and otherwise
 * increments scratchpad byte +1 of the object published at 0x1F800044 while the
 * record's 0x58 group bits are set.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E6D28(void) {
  Battle03LocalWork* work;
  u16 gate;
  u16 status;

  gate = BATTLE_GLOBAL_HALF_62E8;
  if ((gate & 0x400u) != 0u) {
    if (D_801EB4E0->unk_05 == BATTLE_GLOBAL_BYTE_6374) {
      return;
    }
  }

  /* The original loads byte +5 once and reuses it for the range test and both
   * func_801DB524 arguments; the volatile record view emits an extra `andi 0xff`
   * argument conversion, so the record is read through a non-volatile view. */
  work = (Battle03LocalWork*)(void*)D_801EB4E0;
  if (work->unk_05 < 3u) {
    if (func_801DB524(work->unk_05) != 0u) {
      return;
    }
    status = D_80145E90[D_801EB4E0->unk_05].unk_80 & 0x58u;
  } else {
    if (func_801DB524(work->unk_05) != 0u) {
      return;
    }
    status = D_801EB630[D_801EB4E0->unk_05 - 3u].unk_82 & 0x58u;
  }

  if (status != 0u) {
    SPAD_PTR_SLOT(u8, 0x44u)[1]++;
  }
}
