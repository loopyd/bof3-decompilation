#include "bof3/battle/battle03_internal.h"

/* @behavior Drives the local select state: dispatches scratch byte +0x02 through the
 * D_801EB154 handler table, clears the scratch triple +0x5D..+0x5F, then reseeds that
 * triple with 120 or -30 when global bytes +0xE0/+0xE1 select the select phase and the
 * current work byte +0x79 differs from the per-record byte D_80145F09[index*0x140];
 * while global byte +0xE0 is 5 it republishes the work position pair +0x34/+0x38 from
 * D_80143F24/D_80143F28 when flag 0x8000 of D_801462E8 and work word +0x128 bit 1 are
 * both set, otherwise from the D_801EC330 slot-store entry selected by scratch byte
 * +0x05; while the work word +0x80 carries bit 0x20 it nudges that entry's flag byte up
 * (bit 0x800 of work word +0x124 set) or down, toggling bit 0x800 at the 12 / -12 limits
 * and finally republishing work position +0x34 (work byte +0x08 is 1 or 3) or +0x38 from
 * the slot-store words 0x34/0x38 plus the signed flag byte scaled by 0x200; runs
 * localReadyOrHelper1 unless work word +0x124 carries bit 0x1000.
 * @source 0x801DF3F8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DF3F8(void) {
  Battle03LocalWork* clear_work;
  Battle03LocalWork* seed_work;
  Battle03LocalWork* pose_work;
  Battle03LocalWork* place_work;
  Battle03LocalWork* sync_work;
  s8 step;
  u8 count;
  u8 index;

  D_801EB154[battleWork[2]]();

  clear_work = (Battle03LocalWork*)battleWork;
  clear_work->unk_5f = 0u;
  clear_work->unk_5e = 0u;
  clear_work->unk_5d = 0u;

  if (D_801462E0 == 1u) {
    if (D_801462E1[0] >= 2u) {
      if (BATTLE_LOCAL_BYTE_79(BATTLE_LOCAL_WORK_PTR) !=
          D_80145F09[(u32)D_801462EE * 0x140u]) {
        step = -30;
        seed_work = (Battle03LocalWork*)battleWork;
        seed_work->unk_5f = step;
        seed_work->unk_5e = step;
        seed_work->unk_5d = step;
      } else {
        step = 120;
        seed_work = (Battle03LocalWork*)battleWork;
        seed_work->unk_5f = step;
        seed_work->unk_5e = step;
        seed_work->unk_5d = step;
      }
    }
  }

  if (D_801462E0 != 5u) {
    if ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 0x20u) != 0u) {
      if ((BATTLE_LOCAL_WORD_124(BATTLE_LOCAL_WORK_PTR) & 0x0800u) != 0u) {
        index = battleWork[5];
        count = D_801EC330[index].flag_09 + 1;
        D_801EC330[index].flag_09 = count;
        if ((s8)count >= 0xc) {
          D_80146250->unk_124 ^= 0x800u;
        }
      } else {
        index = battleWork[5];
        count = D_801EC330[index].flag_09 - 1;
        D_801EC330[index].flag_09 = count;
        if ((s8)count < -0xb) {
          D_80146250->unk_124 ^= 0x800u;
        }
      }
      pose_work = (Battle03LocalWork*)battleWork;
      if (pose_work->unk_08 == 1u || pose_work->unk_08 == 3u) {
        index = pose_work->unk_05;
        pose_work->unk_34 =
            D_801EC330[index].unk_34 + (((s8)D_801EC330[index].flag_09) << 9);
      } else {
        index = pose_work->unk_05;
        pose_work->unk_38 =
            D_801EC330[index].unk_38 + (((s8)D_801EC330[index].flag_09) << 9);
      }
    }
  } else {
    if ((BATTLE_GLOBAL_HALF_62E8 & 0x8000u) != 0u &&
        (BATTLE_LOCAL_WORD_128(BATTLE_LOCAL_WORK_PTR) & 2u) != 0u) {
      place_work = (Battle03LocalWork*)battleWork;
      place_work->unk_34 = D_80143F24;
      place_work->unk_38 = D_80143F28;
    } else {
      sync_work = (Battle03LocalWork*)battleWork;
      sync_work->unk_34 = D_801EC330[sync_work->unk_05].unk_34;
      sync_work->unk_38 = D_801EC330[sync_work->unk_05].unk_38;
    }
  }

  if ((BATTLE_LOCAL_WORD_124(BATTLE_LOCAL_WORK_PTR) & 0x1000u) == 0u) {
    localReadyOrHelper1();
  }
}
