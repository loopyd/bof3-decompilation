#include "bof3/battle/battle03_internal.h"

/* @behavior clears the key state bytes across all 0x30 queued-slot entries.
 * @source 0x801E5A38
 * @status partial
 * @match 3.64
 * @residual non-exact live audit: 2/45 instructions; 180 original bytes versus 220 current.
 */
void func_801E5A38(void) {
  u8 index;

  index = 0u;
  do {
    u32 offset;

    offset = (u32)index * 0x78u;
    index += 1u;
    BATTLE_SLOT_STORE_BYTE_00(index) = 0u;
    BATTLE_SLOT_STORE_BYTE_05(index) = 0u;
    BATTLE_SLOT_STORE_BYTE_06(index) = 0u;
    BATTLE_SLOT_STORE_BYTE_01(index) = 0u;
    BATTLE_SLOT_STORE_BYTE_02(index) = 0u;
    BATTLE_SLOT_STORE_BYTE_03(index) = 0u;
    BATTLE_SLOT_STORE_BYTE_04(index) = 0u;
    BATTLE_SLOT_STORE_BYTE_48(index) = 0u;
    BATTLE_SLOT_STORE_BYTE_5D(index) = 0u;
    BATTLE_SLOT_STORE_BYTE_5E(index) = 0u;
    BATTLE_SLOT_STORE_BYTE_5F(index) = 0u;
  } while (index < 0x30u);
}
