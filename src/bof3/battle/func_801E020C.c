#include "bof3/battle/battle03_internal.h"

/* @source 0x801E020C
 * @behavior Runs the local readiness helper, then for a scratch work byte +0x02
 * other than 3 with the global 0x80146250 flags +0x80 bit 0x4000 set clears
 * global 0x801462e8 bit 0x40 and leaves 6/4 in scratch work bytes +0x01/+0x02.
 * Otherwise it clears bit 0x40 of 0x801462e8, raising its 0x1000 bit when 0x40
 * was set, and while the 0x80146384 byte has no 0xC0 bits raises the 0x801462e8
 * 0x40 bit when the 0x80146375 byte is 1 and the queued-branch helper agrees, or
 * when that byte is 4, the current kind mask 0x800 is set, the helper agrees and
 * the kind-keyed byte 0x801ca718 + 0x14 * index has no 0x10 bit. It then clears
 * the scratch work pending bit of byte +0x05, masks 0x70 then 0x200 out of the
 * 0x80146250 word +0x124 around the strict local readiness helper (setting
 * 0x80146328 bit 0 when it agrees) and leaves 2/0/0 in scratch work bytes
 * +0x01/+0x02/+0x03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E020C(void) {
  u16* flags;
  u8*  pending_flag;

  localReadyOrHelper2();
  if ((D_1F800044->unk_02 != 3u) &&
      ((D_80146250->unk_80 & 0x4000u) != 0u)) {
    *(u16*)&BATTLE_GLOBAL_HALF_62E8 &= 0xffbfu;
    D_1F800044->unk_01 = 6u;
    D_1F800044->unk_02 = 4u;
  } else {
    flags = (u16*)&BATTLE_GLOBAL_HALF_62E8;
    if ((*flags & 0x40u) == 0u) {
      if ((D_80146384 & 0xc0u) == 0u) {
        if (D_80146375 == 1u) {
          if (checkLocalQueuedBranch() != 0u) {
            *flags |= 0x40u;
          }
        }
        if (D_80146375 == 4u) {
          if ((D_801CA71C[BATTLE_GLOBAL_HALF_63C0].mask_00 & 0x800u) != 0u) {
            if (checkLocalQueuedBranch() != 0u) {
              if ((D_801CA718[(u32)BATTLE_GLOBAL_HALF_63C0 * 0x14u] & 0x10u) ==
                  0u) {
                BATTLE_GLOBAL_HALF_62E8 |= 0x40u;
              }
            }
          }
        }
      }
    } else {
      *flags = (*flags & 0xffbfu) | 0x1000u;
    }
    clearPendingBit(D_1F800044->unk_05);
    D_80146250->unk_124 &= ~0x70u;
    if (func_801E0E0C() != 0u) {
      pending_flag = &D_80146328;
      *pending_flag |= 1u;
    }
    D_80146250->unk_124 &= ~0x200u;
    D_1F800044->unk_01 = 2u;
    D_1F800044->unk_02 = 0u;
  }
  D_1F800044->unk_03 = 0u;
}
