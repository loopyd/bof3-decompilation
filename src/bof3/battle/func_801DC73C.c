#include "bof3/battle/battle03_internal.h"

extern int rand(void);
/* @behavior conditionally zeroes one local status bit after a random gate,
 * otherwise passing through the signed damage value unchanged.
 * @source 0x801DC73C
 * @status partial
 * @match 91.86
 * @residual Live asm diff 79/86 instructions, 344 versus 344 bytes; the only
 * differing hunk is the enemy-record index chain at +0x88, where the original
 * keeps the arg1 - 3 difference in v0 and the x0x118 accumulator in v1 while
 * this build swaps the two registers. Shapes measured without clearing that
 * class: inline enemy subscript, signed index local, record pointer local,
 * re-masked index, byte-pointer enemy view, split difference/scale statements,
 * if/else instead of the goto-joined test, and a two-step index local. The
 * clean-C lever that would steer this is the compiler profile, which this lane
 * is not authorised to search.
 */
u32 func_801DC73C(s16 arg0, u32 arg1, u32 arg2) {

  u32 eidx;
  s16 damage = arg0;
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
