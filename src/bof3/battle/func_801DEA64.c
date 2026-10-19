#include "bof3/battle/battle03_internal.h"

/* @behavior submits the current slot record rooted at `arg0 + 0x74`, optionally
 * appends a script suffix when the global mode latch is set, then queues
 * the common event entry in slot `0`.
 * @source 0x801DEA64
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DEA64(s32 arg0) {
  void* script = &D_801492B8;

  func_801501E4(script, (u32)(arg0 + 0x74), 5u);
  if (BATTLE_GLOBAL_BYTE_63BA != 0u) {
    strcat(script, (const char*)(D_801EB000 + 0xdu));
  }
  func_801DE60C(0u, 1u, 1u, 0u, 0xffu, (u32)script);
}
