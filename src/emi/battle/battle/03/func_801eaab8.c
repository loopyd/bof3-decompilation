#include "internal.h"

/* @behavior refreshes one UI anchor from the panel-task halfwords, then submits a
 * colored UI element using the current ring index.
 * @source 0x801eaab8 FUN_801eaab8
 */
void func_801eaab8(void) {
  func_801d7eb0((s32)(s16)BATTLE_PANEL_TASK_HALF_04,
                (s32)(s16)BATTLE_PANEL_TASK_HALF_06);
  func_8014f800((s16)(BATTLE_PANEL_TASK_HALF_04 + 4u),
                (s16)(BATTLE_PANEL_TASK_HALF_06 + 3u), 0, 0xffu,
                BATTLE_UI_RING_WORD2(BATTLE_UI_RING_INDEX));
}
