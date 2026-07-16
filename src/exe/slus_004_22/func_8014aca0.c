#include "internal.h"

/* @behavior Initializes the runtime work buffers and subsystem state used by
 * the executable's boot loop.
 * @source 0x8014ACA0
 */
void func_8014ACA0(void) {
  u8* work;

  func_8014AD28();
  func_8014AE08();
  work = D_80143D48;
  func_8014AE9C(work);
  func_8014AE9C(work + 0x90);
  func_8014B6B4();
  D_80143D44 = 0;
  D_80143E68 = work;
  func_8014E564(0, 0, 0x400, 0x200);
  DrawSync(0);
  SetDispMask(1);
  func_8014B020();
}
