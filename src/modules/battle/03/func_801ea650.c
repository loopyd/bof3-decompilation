#include "internal.h"

/* does: dispatches the current panel-task icon state, refreshes the UI anchor,
 * then submits the icon selected by panel-task byte `0x0a`.
 * @source: 0x801ea650 FUN_801ea650
 */
void func_801ea650(void) {
  struct PanelTaskIconTable {
    Battle03Handler handlers[3];
  } iconTable = *(struct PanelTaskIconTable const volatile*)0x801d0ff8u;
  u32 slotOffset;

  iconTable.handlers[BATTLE_PANEL_TASK_BYTE_03]();
  func_801d7eb0((s32)(s16)BATTLE_PANEL_TASK_HALF_04,
                (s32)(s16)BATTLE_PANEL_TASK_HALF_06);
  slotOffset = (u32)BATTLE_PANEL_TASK_BYTE_0A * 0xcu;
  func_8014f800((s16)(BATTLE_PANEL_TASK_HALF_04 + 4u),
                (s16)(BATTLE_PANEL_TASK_HALF_06 + 3u),
                *(volatile u8*)(0x801f0000u + slotOffset - 0x4b06u), 0xffu,
                *(volatile u32*)(0x801f0000u + slotOffset - 0x4b0cu));
}
