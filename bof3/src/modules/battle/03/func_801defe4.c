#include "internal.h"

/* does: selects one of several local-work-driven effect ids and forwards it to
 * one of two EXE-side helpers, depending on the current battle-global flags.
 * @source: 0x801defe4 FUN_801defe4
 */
void func_801defe4(void) {
  u16 flags;
  u8  temp;

  if ((BOF3_BATTLE_GLOBAL_BYTE_6325 & 0x20u) == 0u) {
    if ((BOF3_BATTLE_GLOBAL_BYTE_62E0 == 5u) &&
        ((BOF3_BATTLE_GLOBAL_BYTE_6328 & 2u) != 0u)) {
      if ((BOF3_BATTLE_LOCAL_FLAGS_80(BOF3_BATTLE_LOCAL_WORK_PTR) & 0x4000u) ==
          0u) {
        temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x20u;
      } else {
        temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x1cu;
      }
      goto play_queued;
    }
  } else if ((BOF3_BATTLE_GLOBAL_BYTE_62E0 == 5u) &&
             ((BOF3_BATTLE_GLOBAL_BYTE_6328 & 2u) != 0u)) {
    if ((BOF3_BATTLE_LOCAL_FLAGS_80(BOF3_BATTLE_LOCAL_WORK_PTR) & 0x4000u) ==
        0u) {
      func_8014d5f0(BOF3_BATTLE_LOCAL_BYTE_4B(BOF3_BATTLE_LOCAL_SCRATCH_PTR),
                    0x800f0800u, 0x1800);
      return;
    }
    temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x1cu;
  play_queued:
    func_8014d5f0(temp, 0x800f0800u, 0x1800);
    return;
  }

  flags = BOF3_BATTLE_LOCAL_FLAGS_80(BOF3_BATTLE_LOCAL_WORK_PTR);
  if ((flags & 0x4000u) != 0u) {
    temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x1cu;
    goto play_local;
  }

  if ((flags & 0x0800u) != 0u) {
    temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x30u;
    goto play_local;
  }

  if ((BOF3_BATTLE_GLOBAL_HALF_62E8 & 0x10u) != 0u) {
    if ((flags & 4u) == 0u) {
      if ((flags & 0x20c0u) == 0u) {
        temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 8u;
      } else {
        temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x18u;
      }
      goto play_local;
    }
    goto case_5;
  }

  if ((BOF3_BATTLE_LOCAL_WORD_124(BOF3_BATTLE_LOCAL_WORK_PTR) & 2u) != 0u) {
    temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x14u;
    goto play_local;
  }

  switch (BOF3_BATTLE_LOCAL_BYTE_119(BOF3_BATTLE_LOCAL_WORK_PTR)) {
    case 0:
      if ((BOF3_BATTLE_LOCAL_FLAGS_80(BOF3_BATTLE_LOCAL_WORK_PTR) & 0x20c0u) !=
          0u) {
        temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x18u;
        break;
      }
    case 5:
    case_5:
      temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 4u;
      break;
    case 1:
      temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 8u;
      break;
    case 2:
      temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x14u;
      break;
    case 4:
      if ((BOF3_BATTLE_LOCAL_WORD_128(BOF3_BATTLE_LOCAL_WORK_PTR) & 4u) == 0u) {
        if ((BOF3_BATTLE_LOCAL_KIND_MASK(
                 BOF3_BATTLE_LOCAL_HALF_11A(BOF3_BATTLE_LOCAL_WORK_PTR)) &
             0x0800u) == 0u) {
          temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x28u;
        } else {
          temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 8u;
        }
      } else {
        temp = BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x38u;
      }
      break;
    default:
      return;
  }

play_local:
  func_8014d8d4(temp);
}
