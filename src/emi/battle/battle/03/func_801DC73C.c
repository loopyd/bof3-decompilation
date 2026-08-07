#include "internal.h"

extern int rand(void);
/* @behavior conditionally zeroes one local status bit after a random gate,
 * otherwise passing through the signed damage value unchanged.
 * @source 0x801DC73C
 */
u32 func_801DC73C(s16 arg0, u32 arg1, u32 arg2) {
  /*
   * MATCHING_AID: entry-register/allocator residuals proven by asm-diff.
   * damage pins arg0's surviving copy to s2 (original prologue
   * `move s2,a0`; the first return reads a0 directly, the threshold return
   * shifts s2); eidx pins the enemy index `arg1 - 3` to v0 so the
   * multiply chain allocates v1 (original `addiu v0,a1,-3; sll v1,v0,3`
   * sequence at 0x801DC7C4). Clean-C lifetime/declaration/statement-order
   * variants, explicit-copy forms, and a bounded permuter run all stalled
   * at the swapped s1/s2 copies or the v0/v1 chain; the immediately
   * following bin/byte-match was exact. Remove when the allocator ordering
   * is reproduced without pins.
   */
  REGISTER_PIN(u32, eidx, "v0");
  REGISTER_PIN(s16, damage, "s2") = arg0;
  u32 slot = arg2;
  u16 flags;
  s32 threshold;

  if ((BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) {
    return (u32)(s32)arg0;
  }

  arg1 &= 0xffu;
  if (arg1 >= 3u) {
    goto enemy;
  }
  flags = D_80145E90[arg1].unk_80;

test:
  if ((flags & 8u) == 0u) {
    goto threshold;
  }
  if ((rand() & 2u) == 0u) {
    goto threshold;
  }
  goto clear_flag;

enemy:
  eidx = arg1 - 3u;
  flags = D_801EB630[eidx].unk_82;
  goto test;

threshold:
  threshold = D_801EC303;
  if (threshold >= (rand() % 100)) {
    goto clear_flag;
  }
  return (u32)(s32)damage;

clear_flag:
  D_80145E90[slot & 0xffu].unk_120 &= 0xefu;
  return 0u;
}
