#include "internal.h"

/* does: draws the two fixed header sprites and one local label resource when
 * the shared world-ui gate bits are enabled.
 * @source: 0x801f40c4 FUN_801f40c4
 */
void func_801f40c4(s16 arg0, s16 arg1) {
  s16 y;

  if ((BOF3_WORLD00_AREA016_GLOBAL_BYTE_832E & 0x1bu) != 0u) {
    y = arg1;
    func_801f39d8(arg0, y, 4u);
    func_801f39d8((s16)(arg0 + 0x80), y, 5u);
    func_8014f800((s16)(arg0 + 4), (s16)(arg1 + 4), 0, 0xffu,
                  0x80010000u + (u32)BOF3_WORLD00_AREA016_BOOT_HALF_0008);
  }
}
