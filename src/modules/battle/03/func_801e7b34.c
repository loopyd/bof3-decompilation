#include "internal.h"

/* does: reports whether any active alternate local work has a matching active
 * status record in the paired `0x801ec048` array.
 * @source: 0x801e7b34 FUN_801e7b34
 */
u8 func_801e7b34(void) {
  u8 index;

  index = 0u;
  while (index < 3u) {
    if ((BATTLE_LOCAL_ALT_WORK_ARRAY[(u32)index * 0x140u] & 1u) == 0u) {
      return 0u;
    }
    if ((*(volatile u32*)(BATTLE_LOCAL_STATUS_ARRAY + ((u32)index * 0x50u)) &
         1u) != 0u) {
      return 1u;
    }
    index += 1u;
  }
  return 0u;
}
