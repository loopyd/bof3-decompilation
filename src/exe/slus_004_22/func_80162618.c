#include "internal.h"

extern u32         D_80146458;
extern u32         D_8014646C;
extern volatile s8 D_80146489;
extern volatile s8 D_801464A0[];

/* @behavior records the current EMI dispatch handler for the active ring slot,
 * marks that slot active, and advances the loader step.
 * @source 0x80162618
 */
void func_80162618(void) {
  s8*           active_slot;
  volatile u32* dispatches;

  active_slot = (s8*)&D_80146489;
  dispatches = (volatile u32*)(active_slot + 0x2f);
  dispatches[*active_slot] = D_80146458;
  D_801464A0[*active_slot] = 1;

  if (D_8014646C == 0) {
    D_801464A0[*active_slot] = 2;
  }

  D_8014646C += 1;
}
