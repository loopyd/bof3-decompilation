#include "internal.h"

/* does: allocates one slot-store entry and copies the current scratch object's
 * pointer plus words `0x34/0x38/0x3c` into that entry.
 * @source: 0x801ddaf0 FUN_801ddaf0
 */
void func_801ddaf0(void) {
  u32 index;
  u32 offset;

  index = func_801e590c(0u, 6u) & 0xffu;
  offset = index * 0x78u;
  *(volatile u8*)(0x801f0000u + offset - 0x3cc7u) = 0u;
  *(volatile u32*)(0x801f0000u + offset - 0x3c5cu) =
      (u32)BOF3_BATTLE_LOCAL_SCRATCH_PTR;
  *(volatile u32*)(0x801f0000u + offset - 0x3c9cu) =
      BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_34;
  *(volatile u32*)(0x801f0000u + offset - 0x3c98u) =
      BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_38;
  *(volatile u32*)(0x801f0000u + offset - 0x3c94u) =
      *(volatile u32*)((volatile u8*)BOF3_BATTLE_LOCAL_SCRATCH_PTR + 0x3cu);
}
