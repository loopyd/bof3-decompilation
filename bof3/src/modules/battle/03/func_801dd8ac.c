#include "internal.h"

/* does: copies the current local battler's visible values and masked flags into
 * the template record selected by byte `0x13c`.
 * @source: 0x801dd8ac FUN_801dd8ac
 */
void func_801dd8ac(u32 arg0) {
  volatile Battle03LocalWork* battle_work;
  volatile u8*                dst;

  battle_work = &BOF3_BATTLE_LOCAL_WORK_ARRAY[arg0 & 0xffu];
  if ((battle_work->flags_00 & 1u) != 0u) {
    dst =
        (volatile u8*)(0x80144968u +
                       ((u32)BOF3_BATTLE_LOCAL_BYTE_13C(battle_work) * 0xa4u));
    *(volatile u16*)(dst + 0x14) = BOF3_BATTLE_LOCAL_HALF_88(battle_work);
    *(volatile u16*)(dst + 0x16) = BOF3_BATTLE_LOCAL_HALF_8A(battle_work);
    *(volatile u8*)(dst + 0x18) = BOF3_BATTLE_LOCAL_BYTE_8C(battle_work);
    *(volatile u16*)(dst + 0x0c) =
        BOF3_BATTLE_LOCAL_FLAGS_80(battle_work) & 0x60a0u;
    BOF3_BATTLE_LOCAL_FLAGS_80(battle_work) &= 0x60a0u;
  }
}
