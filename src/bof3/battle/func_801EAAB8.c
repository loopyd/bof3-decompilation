#include "bof3/battle/battle03_internal.h"

/* func_8014F800 lives in the main executable and is shared with the scenario,
 * shop and sisyou targets; this target's header has no prototype for it, so the
 * evidenced five-argument form stays file-local. */
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* @source 0x801EAAB8
 * @behavior forwards the panel task position, then emits the UI ring prim for the
 * consumer entry: it reads the panel's halfwords +4/+6, calls func_801D7EB0 with
 * them, and then calls func_8014F800 with the same halfwords offset by +4/+3 (as
 * s16), the constants 0 and 0xFF, and the +4 word of the UI ring entry at the
 * current consumer index.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 32/32 instructions, 128/128 bytes, live byte match.
 */
void func_801EAAB8(void) {
  func_801D7EB0(FIELD_REF(s16, D_80148648, 4), FIELD_REF(s16, D_80148648, 6));
  func_8014F800((s16)(FIELD_REF(u16, D_80148648, 4) + 4u),
                (s16)(FIELD_REF(u16, D_80148648, 6) + 3u), 0, 0xffu,
                uiRingEntries[uiRingHead].unk_04);
}
