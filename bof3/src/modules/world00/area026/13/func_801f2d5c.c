#include "internal.h"

/* does: seeds one local scratch block, then emits four helper-driven slices
 * using the local angle/offset tables.
 * @source: 0x801f2d5c FUN_801f2d5c
 */
void func_801f2d5c(const s32* arg0, s32 arg1) {
  u8 scratch[0x20];
  u8 i;

  func_801afe18(scratch);

  i = 0u;
  do {
    func_801f2e04(arg0, arg1, (s16)(((u32)i & 0x3fu) << 10),
                  BOF3_WORLD00_AREA026_13_TABLE_33FC[i],
                  BOF3_WORLD00_AREA026_13_TABLE_340C[i]);
    i += 1u;
  } while (i < 4u);
}
