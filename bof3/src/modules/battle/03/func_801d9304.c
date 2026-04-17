#include "internal.h"

/* does: initializes one battle-ui state bundle from the caller's byte and the
 * current global mode byte.
 * @source: 0x801d9304 FUN_801d9304
 */
void func_801d9304(u8 arg0) {
  u8  mode;
  u32 ui_mode;

  func_80158db8(1u, 3u);
  mode = BOF3_BATTLE_GLOBAL_BYTE_62F0;
  BOF3_BATTLE_UI_BYTE_8356 = 0u;
  BOF3_BATTLE_UI_BYTE_8357 = arg0;
  BOF3_BATTLE_UI_BYTE_835E = mode;
  ui_mode = BOF3_BATTLE_UI_MODE_TABLE_AF27[mode];
  BOF3_BATTLE_UI_HALF_835A = 0x00f0u;
  BOF3_BATTLE_UI_BYTE_835C = 0u;
  BOF3_BATTLE_UI_BYTE_835D = 0u;
  BOF3_BATTLE_UI_HALF_8358 = ui_mode;
}
