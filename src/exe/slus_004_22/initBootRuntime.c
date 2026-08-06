#include "internal.h"

/* @behavior Initializes the runtime work buffers and subsystem state used by
 * the executable's boot loop.
 * @source 0x8014ACA0
 */
void initBootRuntime(void) {
  u8* work;

  initBootDiscEvents();
  initBootDisplayEnvs();
  work = D_80143D48;
  clearBootOtEntry(work);
  clearBootOtEntry(work + 0x90);
  func_8014B6B4();
  D_80143D44 = 0;
  D_80143E68 = work;
  clearRenderRect(0, 0, 0x400, 0x200);
  DrawSync(0);
  SetDispMask(1);
  func_8014B020();
}
