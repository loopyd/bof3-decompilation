#include "internal.h"

/* @behavior updates the active AREA030 scratch record from the shared world state,
 * or clears scratch state byte `0x02` when the global mode byte is `1`.
 * @source 0x801d11c0 FUN_801d11c0
 */
void func_801d11c0(void) {
  volatile u8* scratch;

  scratch = WORLD00_AREA030_SCRATCH_PTR;

  if (WORLD00_AREA030_GLOBAL_BYTE_5E92 == 1u) {
    scratch[2] = 0u;
    return;
  }

  if (WORLD00_AREA030_GLOBAL_HALF_930E > 0x38) {
    scratch[2] = 3u;
  }

  scratch[0x2au] = WORLD00_AREA030_GLOBAL_BYTE_5EBA;
  scratch[0x49u] = WORLD00_AREA030_GLOBAL_BYTE_5ED9;
  scratch[0x4au] = WORLD00_AREA030_GLOBAL_BYTE_5EDA;
  scratch[0x4bu] = WORLD00_AREA030_GLOBAL_BYTE_5EDB;
  *(volatile u32*)(scratch + 0x50u) = WORLD00_AREA030_GLOBAL_WORD_5EE0;
  *(volatile u32*)(scratch + 0x54u) = WORLD00_AREA030_GLOBAL_WORD_5EE4;
  *(volatile u16*)(scratch + 0x58u) = WORLD00_AREA030_GLOBAL_HALF_5EE8;
  *(volatile u16*)(scratch + 0x5au) = WORLD00_AREA030_GLOBAL_HALF_5EEA;
  func_8014d4e0();
}
