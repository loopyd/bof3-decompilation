#include "bof3/battle/battle03_internal.h"

/* @source 0x801E414C
 * @behavior Runs func_8014DAEC and then either submits the scratch work or
 * re-derives the enemy queued flag before re-arming the scratch work state.
 * While the current enemy work halfword +0x82 carries bit 0x4000 it submits
 * that work through func_80196718, clears bit 0x40 of the global 0x801462E8
 * halfword and leaves 6/4 in scratch work bytes +0x01/+0x02. Otherwise it
 * clears bit 0x40 of that halfword and raises its 0x1000 bit when 0x40 was
 * set; when 0x40 was clear and the 0x80146384 byte has no 0xC0 bits it raises
 * the 0x40 bit when the 0x80146375 byte reads 1 and checkEnemyQueuedPredicate
 * agrees, or when that byte reads 4, the kind mask 0x800 of the 0x801CA71C
 * entry selected by halfword 0x801463C0 is set, the predicate agrees and the
 * kind-keyed byte 0x801CA718 + 0x14 * index has no 0x10 bit. It then clears
 * the scratch work pending bit of byte +0x05, masks 0x70 then 0x200 out of the
 * current enemy work word +0x100 and leaves 2/0 in scratch work bytes
 * +0x01/+0x02. Both paths finally clear scratch work byte +0x03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateEnemyQueuedFlagAndRearmWorkState(void) {
  u16* flags;

  func_8014DAEC();
  if ((battleCurrentEnemyWorkState->unk_82 & 0x4000u) != 0u) {
    func_80196718(D_1F800044);
    *(u16*)&BATTLE_GLOBAL_HALF_62E8 &= 0xffbfu;
    D_1F800044->unk_01 = 6u;
    D_1F800044->unk_02 = 4u;
  } else {
    flags = (u16*)&BATTLE_GLOBAL_HALF_62E8;
    if ((*flags & 0x40u) != 0u) {
      *flags = (*flags & 0xffbfu) | 0x1000u;
    } else if ((D_80146384 & 0xc0u) == 0u) {
      if (D_80146375 == 1u) {
        if (checkEnemyQueuedPredicate() != 0u) {
          *flags |= 0x40u;
        }
      }
      if (D_80146375 == 4u) {
        if ((D_801CA71C[BATTLE_GLOBAL_HALF_63C0].mask_00 & 0x800u) != 0u) {
          if (checkEnemyQueuedPredicate() != 0u) {
            if ((D_801CA718[(u32)BATTLE_GLOBAL_HALF_63C0 * 0x14u] & 0x10u) ==
                0u) {
              BATTLE_GLOBAL_HALF_62E8 |= 0x40u;
            }
          }
        }
      }
    }
    clearPendingBit(D_1F800044->unk_05);
    battleCurrentEnemyWorkState->unk_100 &= 0xffffff8fu;
    battleCurrentEnemyWorkState->unk_100 &= 0xfffffdffu;
    D_1F800044->unk_01 = 2u;
    D_1F800044->unk_02 = 0u;
  }
  D_1F800044->unk_03 = 0u;
}
