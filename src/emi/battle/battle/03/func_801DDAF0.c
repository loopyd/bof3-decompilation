#include "internal.h"

/* @behavior allocates one slot-store entry and copies the current scratch object's
 * pointer plus words `0x34/0x38/0x3c` into that entry.
 * @source 0x801DDAF0
 */
void func_801DDAF0(void) {
  u32 index;
  u32 offset;

  index = func_801E590C(0u, 6u) & 0xffu;
  offset = index * 0x78u;
  BATTLE_SLOT_STORE_FLAG(index) = 0u;
  BATTLE_SLOT_STORE_PTR(index) = (u32)BATTLE_LOCAL_SCRATCH_PTR;
  BATTLE_SLOT_STORE_WORD_34(index) = BATTLE_LOCAL_SCRATCH_PTR->unk_34;
  BATTLE_SLOT_STORE_WORD_38(index) = BATTLE_LOCAL_SCRATCH_PTR->unk_38;
  BATTLE_SLOT_STORE_WORD_3C(index) =
      *(volatile u32*)((volatile u8*)BATTLE_LOCAL_SCRATCH_PTR + 0x3cu);
}
