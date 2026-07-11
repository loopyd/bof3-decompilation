#include "internal.h"

/* @behavior selects one of several local-work-driven effect ids and forwards it to
 * one of two EXE-side helpers, depending on the current battle-global flags.
 * @source 0x801defe4 FUN_801defe4
 */
void func_801defe4(void) {
  u16 flags;
  u8  temp;

  if ((BATTLE_GLOBAL_BYTE_6325 & 0x20u) == 0u) {
    if ((BATTLE_GLOBAL_BYTE_62E0 == 5u) &&
        ((BATTLE_GLOBAL_BYTE_6328 & 2u) != 0u)) {
      if ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 0x4000u) == 0u) {
        temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x20u;
      } else {
        temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x1cu;
      }
      goto play_queued;
    }
  } else if ((BATTLE_GLOBAL_BYTE_62E0 == 5u) &&
             ((BATTLE_GLOBAL_BYTE_6328 & 2u) != 0u)) {
    if ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 0x4000u) == 0u) {
      func_8014d5f0(BATTLE_LOCAL_BYTE_4B(BATTLE_LOCAL_SCRATCH_PTR), 0x800f0800u,
                    0x1800);
      return;
    }
    temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x1cu;
  play_queued:
    func_8014d5f0(temp, 0x800f0800u, 0x1800);
    return;
  }

  flags = BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR);
  if ((flags & 0x4000u) != 0u) {
    temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x1cu;
    goto play_local;
  }

  if ((flags & 0x0800u) != 0u) {
    temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x30u;
    goto play_local;
  }

  if ((BATTLE_GLOBAL_HALF_62E8 & 0x10u) != 0u) {
    if ((flags & 4u) == 0u) {
      if ((flags & 0x20c0u) == 0u) {
        temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 8u;
      } else {
        temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x18u;
      }
      goto play_local;
    }
    goto case_5;
  }

  if ((BATTLE_LOCAL_WORD_124(BATTLE_LOCAL_WORK_PTR) & 2u) != 0u) {
    temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x14u;
    goto play_local;
  }

  switch (BATTLE_LOCAL_BYTE_119(BATTLE_LOCAL_WORK_PTR)) {
    case 0:
      if ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 0x20c0u) != 0u) {
        temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x18u;
        break;
      }
    case 5:
    case_5:
      temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 4u;
      break;
    case 1:
      temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 8u;
      break;
    case 2:
      temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x14u;
      break;
    case 4:
      if ((BATTLE_LOCAL_WORD_128(BATTLE_LOCAL_WORK_PTR) & 4u) == 0u) {
        if ((BATTLE_LOCAL_KIND_MASK(
                 BATTLE_LOCAL_HALF_11A(BATTLE_LOCAL_WORK_PTR)) &
             0x0800u) == 0u) {
          temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x28u;
        } else {
          temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 8u;
        }
      } else {
        temp = BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x38u;
      }
      break;
    default:
      return;
  }

play_local:
  func_8014d8d4(temp);
}
