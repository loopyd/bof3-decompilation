#include "internal.h"

/* does: refreshes each active local battler's display template fields from the
 * template table selected by byte `0x13c`, then rebuilds the local display
 * data.
 * @source: 0x801e0b64 FUN_801e0b64
 */
void func_801e0b64(void) {
  const u8* template_base;
  u8        index;

  template_base =
      (const u8*)(0x80144968u +
                  ((u32)BATTLE_LOCAL_BYTE_13C(BATTLE_LOCAL_WORK_PTR) * 0xa4u));
  func_80164a44((u32)template_base);

  index = 0u;
  while (index < BATTLE_GLOBAL_BYTE_62F0) {
    volatile u8* local_bytes;
    const u8*    src_bytes;
    u32          offset;

    local_bytes = (volatile u8*)BATTLE_LOCAL_WORK_ARRAY + ((u32)index * 0x140u);
    src_bytes =
        (const u8*)(0x80144968u + ((u32)BATTLE_LOCAL_BYTE_13C(
                                       &BATTLE_LOCAL_WORK_ARRAY[index]) *
                                   0xa4u));

    for (offset = 0; offset < 6u; offset += 1u) {
      local_bytes[0x82u + offset] = src_bytes[0x0eu + offset];
    }
    for (offset = 0; offset < 0x20u; offset += 1u) {
      local_bytes[0x90u + offset] = src_bytes[0x1cu + offset];
    }
    index += 1u;
  }

  func_800aaa74();

  index = 0u;
  while (index < BATTLE_GLOBAL_BYTE_62F0) {
    volatile u8* local_bytes;
    u32          offset;

    local_bytes = (volatile u8*)BATTLE_LOCAL_WORK_ARRAY + ((u32)index * 0x140u);
    for (offset = 0; offset < 0x20u; offset += 1u) {
      local_bytes[0x90u + offset] = local_bytes[0xb0u + offset];
    }
    index += 1u;
  }

  func_800a4458();

  index = 0u;
  while (index < BATTLE_GLOBAL_BYTE_62F0) {
    func_800a9bd8(index);
    index += 1u;
  }
}
