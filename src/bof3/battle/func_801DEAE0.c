#include "bof3/battle/battle03_internal.h"

extern int rand(void);

/* @behavior chooses one event id from the small fixed table based on the current
 * halfword mode at `0x801463d0`, then writes that event into slot `0`.
 * @source 0x801DEAE0
 * @status partial
 * @match 78.95
 * @residual live asm diff 45/57 instructions, 228 versus 228 bytes. First mismatch +0x34: the original branches forward into the mode-1 arm (beq mode,1) which sits behind the dispatch chain, then jumps to the 2..4 arm; this build inverts the test, inlines the mode-1 arm and re-derives the rest of the chain afterwards, and it copies the resolved id into v1 because it materialises the 0xFF argument constant in v0. Both the switch and the inverted-nested shapes reach the same 45/57 live score, and a flag-search over the 52 reviewed profiles left the canonical -O2 profile the best (no exact match), so the residual is the dispatch layout and its register assignment, not a compiler profile.
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
