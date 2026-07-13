#include "internal.h"

/* @behavior Initializes the runtime work buffers and subsystem state used by
 * the executable's boot loop.
 * @source 0x8014aca0 func_8014aca0
 */
void func_8014aca0(void) {
  u8* work;

  func_8014ad28();
  func_8014ae08();
  work = DAT_80143d48;
  func_8014ae9c(work);
  func_8014ae9c(work + 0x90);
  func_8014b6b4();
  DAT_80143d44 = 0;
  DAT_80143e68 = work;
  func_8014e564(0, 0, 0x400, 0x200);
  DrawSync(0);
  SetDispMask(1);
  func_8014b020();
}
