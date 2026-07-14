#include "internal.h"

extern u32 func_8014d8d4(u8 arg0);

#define REMAP_PAGE ((volatile u8*)0x801d0000)

/* @behavior When the world/front flags bit 0 is set, remaps arg0 through
 * a 2-byte-per-entry lookup table at 0x801cd06c: calls func_8014d8d4
 * with the first table byte and writes the second byte to the scratchpad
 * work area's unk_2A field. When the flag is clear, calls func_8014d8d4
 * with arg0 directly.
 * @source 0x801b5bdc original_label
 */
void func_801b5bdc(u8 arg0) {
  u32 index;

  if (!(DAT_80143f02 & 1)) {
    func_8014d8d4(arg0);
    return;
  }
  index = (u32)arg0 * 2;
  func_8014d8d4(REMAP_PAGE[index - 0x2f94]);
  SCRATCH_WORK->unk_2A = REMAP_PAGE[index - 0x2f93];
}
