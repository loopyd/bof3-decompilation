#include "bof3/battle/battle03_internal.h"

extern int rand(void);

/* Eight-byte percent band the body copies into its own frame from the overlay's
 * leading header bytes: four 0x32 entries, three 0x3C and one 0x46. */
typedef struct Battle03DamageBand {
  u8 percent[8];
} Battle03DamageBand;

extern const Battle03DamageBand D_801D0C7C; /* @source 0x801D0C7C @kind table */
/* Defence accumulator the attacker guard scale feeds. */
extern u16 D_801EC30C; /* @source 0x801EC30C @kind data */
/* Kind records selected by the attacker's kind byte, 0x18-byte stride; the body
 * stores the selected record's leading byte into the scratchpad. */
extern u8 D_801C90EB[]; /* @source 0x801C90EB @kind data */
/* Shared status halfword whose 0x40 and 0x20 bits record the target gates this
 * body applies. */
extern u16 D_801463DA; /* @source 0x801463DA @kind data */

extern u32 resolveEnemyTargetDamageThroughGates(s16 damage, u8 attacker,
                                                u8 target);

/* @source 0x801DC044
 * @behavior Resolves the damage amount one attacker deals to one target. It
 *   first seeds the scratchpad damage-kind halfword 0x1F800000: with the
 *   caller's own kind mask when that mask is not 0xFFFF, otherwise, for a party
 *   attacker, with the leading byte of the 0x18-byte kind record selected by
 *   the attacker's own kind byte, and with zero for an enemy attacker. When the
 *   attacker's +0x128 flag word carries bit 0x40 it then scales the defence
 *   accumulator 0x801EC30C by the attacker's guard byte +0x138 (adding half of
 *   the product for a party attacker, storing only half of it for an enemy
 *   attacker), clears that guard byte and clears the flag. The base amount
 *   comes from func_801DCAD8 with the zero damage mode; a party attacker then
 *   doubles it when its kind byte is 0x13, 0x17 or 0x33 and the enemy target's
 *   kind byte +0x7D is 4, and again when its kind byte is 0x16, 0x35 or 0x40
 *   and that byte is 1. The target's status halfword (local +0x80, enemy +0x82)
 *   drives the remaining gates: while either of bits 0x40 and 0x20 is set it
 *   raises the matching bit of 0x801463DA, clears the target's guard bits
 *   through func_800A36F0 and zeroes the target's +0x121/+0xFD byte, and bit
 *   0x04 as well as both of those arms raise a zero amount to one; with none of
 *   bits 0x64 set the amount instead passes through func_801DC73C for a party
 *   target or resolveEnemyTargetDamageThroughGates for an enemy target. A
 *   target guard byte carrying bit 0x02 rolls the frame band by rand() % 8 and
 *   keeps rand()'s percentage of the amount. While 0x801462E8 carries bit 0x80
 *   the amount grows to twice itself plus a quarter of the second
 *   func_801DCAD8 answer (mode 1). A party target subtracts a quarter of itself
 *   while 0x80144F58 reads 2, and target status bit 0x80 subtracts the same
 *   quarter; bits 0x60 and the full-damage flag each raise a zero amount to
 *   one. Finally the target's guard word at +0x124 (party) or +0x100 (enemy)
 *   zeroes the amount on bit 0x10000, and the result is clamped to 9999 and
 *   -9999 and returned sign-extended.
 * @status partial
 * @match 84.55
 * @residual Live native diff 383/453 instructions, 1812 versus 1784 bytes;
 *   first mismatch +0x00A4, the fourth `andi v1,s4,0xFF` that the store arm of
 *   the kind-mask test recomputes where the original keeps the value the branch
 *   delay slot already produced, which shifts every later branch displacement.
 *   The measured classes: the masked attacker and target bytes are
 *   re-materialised at three joins, the Phase D doubling compiles to
 *   `andi 0xFFFF`/`sll 1` instead of the original's `sll 16`/`srl 15`, the dead
 *   `addiu v1,v1,-3` the original keeps in one delay slot is absent, and two
 *   byte/index expressions pick the other scratch register. The body, statement
 *   order and every recorded cell access match; the residual is allocation and
 *   rematerialisation, not representation.
 */
s32 resolveBattlerDamageAmount(u8 attacker, u8 target, u16 kind_mask) {
  Battle03DamageBand band;
  s16 damage;
  u16 status;
  u8 guard;
  u8 kind;

  band = D_801D0C7C;
  if (kind_mask == 0xffffu) {
    if (attacker < 3u) {
      D_1F800000 = D_801C90EB[D_80145E90[attacker].unk_82 *
                              0x18];
    } else {
      D_1F800000 = 0;
    }
  } else {
    D_1F800000 = kind_mask;
  }

  if (attacker < 3u) {
    if ((D_80145FB8[attacker].flags_00 & 0x40u) != 0u) {
      u16* acc = &D_801EC30C;
      *acc += *acc * D_80145E90[attacker].unk_138 / 2;
      D_80145FB8[attacker].flags_00 &= 0xffffffbfu;
      D_80145E90[attacker].unk_138 = 0;
    }
  } else {
    if ((D_801EB734[attacker - 3u].word_00 & 0x40u) != 0u) {
      u16* acc = &D_801EC30C;
      *acc = *acc * D_801EB630[attacker - 3u].unk_114 / 2;
      D_801EB734[attacker - 3u].word_00 &= 0xffffffbfu;
      D_801EB630[attacker - 3u].unk_114 = 0;
    }
  }

  damage = func_801DCAD8(attacker, target, 0);

  if (attacker < 3u) {
    if (target >= 3u) {
      kind = D_80145E90[attacker].unk_82;
      if ((kind == 0x13u) || (kind == 0x17u) || (kind == 0x33u)) {
        if (D_801EB630[target - 3u].unk_7d == 4u) {
          damage = (u16)damage * 2;
        }
      }
      kind = D_80145E90[attacker].unk_82;
      if ((kind == 0x16u) || (kind == 0x35u) || (kind == 0x40u)) {
        if (D_801EB630[target - 3u].unk_7d == 1u) {
          damage = (u16)damage * 2;
        }
      }
    }
  }

  if (target < 3u) {
    status = D_80145E90[target].unk_80;
    guard = *(u8*)&D_80145E90[target].unk_124;
  } else {
    status = D_801EB630[target - 3u].unk_82;
    guard = *(u8*)&D_801EB630[target - 3u].unk_100;
  }

  if ((status & 0x64u) != 0u) {
    if ((status & 0x40u) != 0u) {
      u16* gate = &D_801463DA;
      *gate |= 0x40u;
      if (target < 3u) {
        func_800A36F0(D_80145E90[target].unk_05, 0x40u);
        D_80145E90[target].unk_121 = 0;
      } else {
        func_800A36F0(D_801EB630[target - 3u].unk_05, 0x40u);
        D_801EB630[target - 3u].unk_fd = 0;
      }
      if (damage == 0) {
        damage = 1;
      }
    }
    if ((status & 0x20u) != 0u) {
      u16* gate = &D_801463DA;
      *gate |= 0x20u;
      if (target < 3u) {
        func_800A36F0(D_80145E90[target].unk_05, 0x20u);
        D_80145E90[target].unk_121 = 0;
      } else {
        func_800A36F0(D_801EB630[target - 3u].unk_05, 0x20u);
        D_801EB630[target - 3u].unk_fd = 0;
      }
      if (damage == 0) {
        damage = 1;
      }
    }
    if (((status & 0x04u) != 0u) && (damage == 0)) {
      damage = 1;
    }
  } else {
    if (target < 3u) {
      damage = func_801DC73C(damage, attacker, target);
    } else {
      damage = resolveEnemyTargetDamageThroughGates(damage, attacker, target);
    }
  }

  if ((guard & 0x02u) != 0u) {
    damage = damage * band.percent[rand() % 8] / 100;
  }

  if ((BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) {
    damage = damage * 2 + (s16)func_801DCAD8(attacker, target, 1) / 4;
  }

  if ((target < 3u) && (PSX_REF(u8, 0x80144f58u) == 2u)) {
    damage -= damage >> 2;
  }
  if ((status & 0x80u) != 0u) {
    damage -= damage >> 2;
  }
  if (((status & 0x60u) != 0u) && (damage == 0)) {
    damage = 1;
  }
  if (((BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) && (damage == 0)) {
    damage = 1;
  }

  if (target < 3u) {
    if ((D_80145E90[target].unk_124 & 0x10000u) != 0u) {
      damage = 0;
    }
  } else {
    if ((D_801EB630[target - 3u].unk_100 & 0x10000u) != 0u) {
      damage = 0;
    }
  }
  if (damage >= 10000) {
    damage = 9999;
  }
  if (damage < -9999) {
    damage = -9999;
  }
  return damage;
}
