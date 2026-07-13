#include "internal.h"

extern u32 DAT_80146458;
extern u32 DAT_8014646c;
extern vs8 DAT_80146489;
extern vs8 DAT_801464a0[];

/* @behavior records the current EMI dispatch handler for the active ring slot,
 * marks that slot active, and advances the loader step.
 * @source 0x80162618 FUN_80162618
 */
void func_80162618(void) {
  s8*           active_slot;
  volatile u32* dispatches;

  active_slot = (s8*)&DAT_80146489;
  dispatches = (volatile u32*)(active_slot + 0x2f);
  dispatches[*active_slot] = DAT_80146458;
  DAT_801464a0[*active_slot] = 1;

  if (DAT_8014646c == 0) {
    DAT_801464a0[*active_slot] = 2;
  }

  DAT_8014646c += 1;
}
