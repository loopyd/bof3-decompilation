#include "bof3/battle/battle03_internal.h"

extern int rand(void);
extern u8 D_801EC324; /* @source 0x801EC324 @kind data */

/* @source 0x801DC894
 * @behavior Returns the signed value arg0 unchanged while the full-damage flag
 *   0x80 of the battle global halfword 0x801462E8 is set, while the party work
 *   record selected by the low byte of arg1 carries flag 0x100 in its
 *   0x80145FB8 word, or when that low byte lands in the enemy band. The
 *   attacker gate reads bit 8 of the +0x80 halfword of the local work record
 *   selected by the low byte of arg1, or of the +0x82 halfword of the enemy
 *   work record selected by that byte minus 3, and cancels the target on a
 *   nonzero rand() & 2. The target gate reads bit 4 of the +0x80 halfword of
 *   the enemy work record selected by the low byte of arg2 minus 3 and cancels
 *   on a nonzero rand() & 2; with that gate clear a party attacker also cancels
 *   unless rand() % 100 stays below the chance byte 0x801EC324, and an enemy
 *   attacker returns arg0. Cancelling clears bit 0x10 of the +0xFC pending byte
 *   of that same enemy work record and returns zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 resolveEnemyTargetDamageThroughGates(s16 arg0, u8 arg1, u8 arg2) {
  u32 target;
  s32 chance;

  if ((BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) {
    return (u32)(s32)arg0;
  }
  if (arg1 < 3u) {
    if ((D_80145E90[arg1].unk_80 & 8u) == 0u) {
      goto stage2;
    }
    if ((rand() & 2u) == 0u) {
      goto stage2;
    }
    goto cancel;
  }
  if ((D_801EB630[arg1 - 3u].unk_82 & 8u) == 0u) {
    goto stage2;
  }
  if ((rand() & 2u) != 0u) {
    goto cancel;
  }
stage2:
  if (arg1 < 3u) {
    if ((D_80145FB8[arg1].flags_00 & 0x100u) != 0u) {
      goto keep;
    }
  }
  target = arg2 & 0xffu;
  if ((D_801EB630[target - 3u].unk_80 & 4u) != 0u) {
    if ((rand() & 2u) != 0u) {
      D_801EB630[target - 3u].unk_fc &= 0xefu;
      return 0u;
    }
  }
  if (arg1 >= 3u) {
    goto keep;
  }
  chance = D_801EC324;
  if ((s32)(rand() % 100) < chance) {
    goto keep;
  }
cancel:
  D_801EB630[(arg2 & 0xffu) - 3u].unk_fc &= 0xefu;
  return 0u;
keep:
  return (u32)(s32)arg0;
}
