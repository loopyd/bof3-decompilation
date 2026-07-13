#include "internal.h"

extern u32    DAT_80143ea0;
extern u32    DAT_80143ea4;
extern u_long DAT_80143db8;

/* @behavior loads and transfers control to LOGO.EXE, then restores the boot
 * rendering path after the loaded executable returns.
 * @source 0x8014aee0 FUN_8014aee0
 */
void func_8014aee0(void) {
  u_long* ordering_table;
  u32*    exec_state;

  func_8014e0fc(s__LOGO_LOGO_EXE_1_80149800);

  exec_state = &DAT_80143ea0;
  *exec_state = 0x801FF000;
  DAT_80143ea4 = 0;

  StopCallback();
  PadStop();
  func_8017e0b4();
  func_8017ee0c();
  Exec((struct EXEC*)(exec_state - 8), 0, 0);
  func_8017ee1c();
  func_8014ad28();

  ordering_table = &DAT_80143db8;
  ClearOTag(ordering_table, 8);
  DrawOTag(ordering_table);
  ordering_table += 0x24;
  ClearOTag(ordering_table, 8);
  DrawOTag(ordering_table);

  func_8014e564(0, 0, 0x400, 0x200);
  DrawSync(0);
  SetDispMask(1);
}
