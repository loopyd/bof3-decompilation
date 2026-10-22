#include "bof3/battle/battle03_internal.h"

extern int rand(void);

/* @behavior chooses one event id from the small fixed table based on the current
 * word mode at `0x801463d0`, then writes that event into slot `0`.
 * @source 0x801DEAE0
 * @status partial
 * @match 78.95
 * @residual live asm diff 45/57 instructions, 228 versus 228 bytes. First
 * mismatch +0x34: the original branches forward into the mode-1 arm (beq mode,1)
 * and fills its delay slot with `addiu s0,sp,0x18`, the base of `table`; this
 * build schedules that dependency-free addiu at +0x34, four instructions before
 * the original's +0x44 delay slot, and then takes the register tie-break on the
 * 0xFF argument (constant in v0 here, in v1 there, so one extra move). No
 * measured clean-C shape improves on 45/57: select in the if
 * and one call after the arms (44/57, 224 bytes), the same with a named pointer
 * (39/57), arm-3-first else-if with one call and with per-arm calls (42/57, the
 * second 232 bytes), arm-1 moved to the else with inner nesting in both
 * polarities (36/57 and 40/57), and in the repair round the intact
 * `table = local_18` statement deferred after the mode load, inside the mode-1
 * arm, or ahead of the mode load (45/57 and the same emitted bytes each time,
 * which shows the statement position does not control the hoist). The 52-profile
 * flag-search that left canonical -O2 best predates and sits outside this lane,
 * which was restricted to asm-diff, sweep and the combined gate.
 */
void func_801DEAE0(void) {
  u8          local_18[6];
  u32         mode;
  u32         id;
  u8*         table = local_18;

  __builtin_memcpy(local_18, D_801D0C98, sizeof(local_18));
  mode = D_801463D0;

  if (mode == 1u) {
    id = func_801502D0(table[rand() & 1]);
  } else if ((mode != 0u) && (mode < 5u)) {
    id = func_801502D0(local_18[2 + (rand() & 1)]);
  } else {
    id = func_801502D0(local_18[4 + (rand() & 1)]);
  }
  func_801DE60C(0u, 2u, 0u, 0u, 0xffu, id);
}
