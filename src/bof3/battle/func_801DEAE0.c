#include "bof3/battle/battle03_internal.h"

extern int rand(void);

/* @behavior chooses one event id from the small fixed table based on the current
 * halfword mode at `0x801463d0`, then writes that event into slot `0`.
 * @source 0x801DEAE0
 * @status partial
 * @match 78.95
 * @residual non-exact live audit: 45/57 instructions; 228 original bytes versus 228 current.
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
