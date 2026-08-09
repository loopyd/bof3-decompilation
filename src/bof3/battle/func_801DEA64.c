#include "bof3/battle/battle03_internal.h"

/* @behavior submits the current slot record rooted at `arg0 + 0x74`, optionally
 * refreshes one followup script when the global mode latch is set, then queues
 * the common event entry in slot `0`.
 * @source 0x801DEA64
 * @status partial
 * @match 58.06
 * @residual non-exact live audit: 18/31 instructions; 124 original bytes versus 120 current.
 */
void func_801DEA64(s32 arg0) {
  func_801501E4(BATTLE_SCRIPT_TABLE_492B8, (u32)(arg0 + 0x74), 5u);
  if (BATTLE_GLOBAL_BYTE_63BA != 0u) {
    strcat(BATTLE_SCRIPT_TABLE_492B8, (const char*)BATTLE_SCRIPT_TABLE_0B00D);
  }
  func_801DE60C(0u, 1u, 1u, 0u, 0xffu, (u32)BATTLE_SCRIPT_TABLE_492B8);
}
