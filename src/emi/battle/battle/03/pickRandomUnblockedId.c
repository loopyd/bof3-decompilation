#include "internal.h"

extern int rand(void);
/* @behavior builds a small list of selectable ids from `3..10`, excluding blocked
 * entries and the requested id, then returns one random surviving choice.
 * @source 0x801E29B4
 */
u8 pickRandomUnblockedId(u8 arg0) {
  u8  choices[8];
  u8  count;
  u8  slot;
  s32 random_value;

  count = 0u;
  slot = 3u;
  do {
    if ((func_801DB524(slot) == 0u) && (arg0 != slot)) {
      choices[count] = slot;
      count += 1u;
    }
    slot += 1u;
  } while (slot < 0x0bu);

  random_value = rand();

  return choices[random_value % count];
}
