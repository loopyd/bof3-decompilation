#include "internal.h"

extern u32    D_80143EA0;
extern u32    D_80143EA4;
extern u_long D_80143DB8;

/* @behavior loads and transfers control to LOGO.EXE, then restores the boot
 * rendering path after the loaded executable returns.
 * @source 0x8014AEE0
 */
void func_8014AEE0(void) {
  u_long* ordering_table;
  u32*    exec_state;

  func_8014E0FC(s__LOGO_LOGO_EXE_1_80149800);

  exec_state = &D_80143EA0;
  *exec_state = 0x801FF000;
  D_80143EA4 = 0;

  StopCallback();
  PadStop();
  func_8017E0B4();
  func_8017EE0C();
  Exec((struct EXEC*)(exec_state - 8), 0, 0);
  func_8017EE1C();
  func_8014AD28();

  ordering_table = &D_80143DB8;
  ClearOTag(ordering_table, 8);
  DrawOTag(ordering_table);
  ordering_table += 0x24;
  ClearOTag(ordering_table, 8);
  DrawOTag(ordering_table);

  func_8014E564(0, 0, 0x400, 0x200);
  DrawSync(0);
  SetDispMask(1);
}
