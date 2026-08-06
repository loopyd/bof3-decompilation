#include "internal.h"

/* @behavior copies the queued halfwords into the active panel-task slots, clears
 * the active flag byte, and backs the state byte up by one.
 * @source 0x801EA1A4
 */
void battle03_panel_task_commit_queued(void) {
  u8* volatile* root;
  u8*           temp_a1;
  u8*           temp_v0;
  u16           value_10;
  u16           value_12;

  root = BATTLE_PANEL_ROOT_BASE;
  temp_v0 = root[-0x1e6e];
  value_10 = *(volatile u16*)(temp_v0 + 0x10);
  value_12 = *(volatile u16*)(temp_v0 + 0x12);
  temp_v0[0xf] = 0u;
  temp_a1 = root[-0x1e6e];
  *(volatile u16*)(temp_v0 + 4) = value_10;
  *(volatile u16*)(temp_v0 + 6) = value_12;
  temp_a1[3] = (u8)(temp_a1[3] - 1u);
}
