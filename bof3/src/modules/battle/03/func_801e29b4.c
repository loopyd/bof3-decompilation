#include "internal.h"

/* does: builds a small list of selectable ids from `3..10`, excluding blocked
 * entries and the requested id, then returns one random surviving choice.
 * @source: 0x801e29b4 FUN_801e29b4
 */
u8 func_801e29b4(u8 arg0) {
  u8  choices[8];
  u8  count;
  u8  slot;
  s32 random_value;

  count = 0u;
  slot = 3u;
  do {
    if ((func_801db524(slot) == 0u) && (arg0 != slot)) {
      choices[count] = slot;
      count += 1u;
    }
    slot += 1u;
  } while (slot < 0x0bu);

  random_value = func_8017e3d4();

  return choices[random_value % count];
}
