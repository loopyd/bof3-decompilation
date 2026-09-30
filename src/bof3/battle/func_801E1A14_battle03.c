#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1A14
 * @behavior While the current local work byte +0x119 is 4, folds its +0x8a
 * halfword down by the global 0x801463c8 byte, raises global 0x801462e8 bit
 * 0x800 and clears that mode byte. Then selects the current kind from global
 * halfword 0x801463c0: when that kind's 0x800 mask is set it advances scratch
 * work byte +0x02, otherwise it runs the second local readiness helper and
 * stores 4 into that byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E1A14(void) {
  Battle03LocalWork* work;
  u16*               flags;
  u16                index;

  work = D_80146250;
  if (work->unk_119 == 4u) {
    work->unk_8a -= BATTLE_GLOBAL_BYTE_63C8;
  }
  flags = (u16*)&BATTLE_GLOBAL_HALF_62E8;
  *flags |= 0x800u;
  D_80146250->unk_119 = 0u;
  index = BATTLE_GLOBAL_HALF_63C0;
  if (D_801CA71C[index].mask_00 & 0x800u) {
    D_1F800044->unk_02++;
  } else {
    localReadyOrHelper2();
    D_1F800044->unk_02 = 4;
  }
}
