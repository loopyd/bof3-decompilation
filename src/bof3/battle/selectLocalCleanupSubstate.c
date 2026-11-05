#include "bof3/battle/battle03_internal.h"

extern u8 battleByte80144979[]; /* @source 0x80144979 @kind unknown */
extern u8 battleByte8014497a[]; /* @source 0x8014497A @kind unknown */
extern u8 battleByte8014497b[]; /* @source 0x8014497B @kind unknown */
extern u8 battleByte801462f1; /* @source 0x801462F1 @kind unknown */
extern u8 battleByte801463ca; /* @source 0x801463CA @kind unknown */

/* @source 0x801E0788
 * @behavior Clears the current battler's action flags and selects a cleanup
 * substate from local flags and identity bytes. The ordinary path decrements
 * the shared count, clears local transient bytes and flags, tests readiness,
 * and either selects substate 4 or resets the scratch state to (3,0,0).
 * Every path finishes with localReadyOrHelper1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectLocalCleanupSubstate(void) {
  Battle03LocalWork* work;
  Battle03LocalWork* resetWork;
  Battle03LocalWork* finalWork;
  u16* earlyFlags;
  u16* lateFlags;
  u8* count;
  u8* ready;
  u8 index;

  func_801DDFEC(D_1F800044->unk_05);
  clearBattlerActionFlags(D_1F800044->unk_05);
  work = D_80146250;
  if ((work->unk_124 & 4u) != 0u) {
    D_1F800044->unk_09 = 30u;
    D_1F800044->unk_03 = 1u;
    D_1F800044->unk_04 = 0u;
  } else if ((work->unk_128 & 2u) != 0u) {
    earlyFlags = (u16*)&BATTLE_GLOBAL_HALF_62E8;
    if ((*earlyFlags & 0x4000u) != 0u &&
        battleByte801463ca == D_1F800044->unk_05) {
      *earlyFlags &= 0xbfffu;
    }
    goto substate4;
  } else if (work->unk_86 == 0x18u) {
    battleByte8014497a[work->pad_13a[2] * 0xa4] = 0u;
    D_1F800044->unk_09 = 30u;
    D_1F800044->unk_03 = 3u;
  } else if (work->unk_87 == 0x18u) {
    battleByte8014497b[work->pad_13a[2] * 0xa4] = 0u;
    D_1F800044->unk_09 = 30u;
    D_1F800044->unk_03 = 6u;
  } else if (work->unk_85 == 0x43u) {
    battleByte80144979[work->pad_13a[2] * 0xa4] = 0u;
    D_1F800044->unk_09 = 30u;
    D_1F800044->unk_03 = 5u;
  } else {
    count = &battleByte801462f1;
    --*count;
    index = 0u;
    do {
      D_80146250->pad_12c[index] = 0u;
      ++index;
    } while (index < 8u);
    resetWork = D_80146250;
    resetWork->unk_121 = 0u;
    resetWork->unk_128 &= 0xfffb84ffu;
    D_80146250->unk_136 = 0u;
    D_80146250->unk_137 = 0u;
    D_80146250->unk_138 = 0u;
    D_80146250->unk_139 = 0u;
    func_800A9BD8(D_1F800044->unk_05);
    if (func_801E0E0C() != 0u || battleByte801462f1 == 0u) {
      ready = &D_80146328;
      *ready |= 1u;
    }
    lateFlags = (u16*)&BATTLE_GLOBAL_HALF_62E8;
    if ((*lateFlags & 0x4000u) != 0u &&
        battleByte801463ca == D_1F800044->unk_05) {
      *lateFlags &= 0xbfffu;
    }
    if ((D_80146250->unk_128 & 1u) != 0u) {
substate4:
      D_1F800044->unk_03 = 4u;
    } else {
      clearPendingBit(D_1F800044->unk_05);
      finalWork = D_80146250;
      finalWork->unk_124 &= ~0x70u;
      finalWork->unk_124 &= ~0x200u;
      D_1F800044->unk_01 = 3u;
      D_1F800044->unk_02 = 0u;
      D_1F800044->unk_03 = 0u;
    }
  }
  localReadyOrHelper1();
}
