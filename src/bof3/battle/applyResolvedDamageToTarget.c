#include "bof3/battle/battle03_internal.h"

/* Cells of the shared main executable that only this body reaches, so each
 * keeps the fixed address the original spells with its own lui/addu pair. */
extern u8 D_80145FB0[]; /* @source 0x80145FB0 @kind unknown */
extern u8 D_80145F12[]; /* @source 0x80145F12 @kind unknown */
extern u8 D_801EB6C4[]; /* @source 0x801EB6C4 @kind unknown */
extern u8 D_801EB6D0[]; /* @source 0x801EB6D0 @kind unknown */
extern u8 D_801EB72C[]; /* @source 0x801EB72C @kind unknown */
extern u8 D_801EB730[]; /* @source 0x801EB730 @kind unknown */
extern u8 D_801EB075[]; /* @source 0x801EB075 @kind unknown */

/* @source 0x801DBB78
 * @behavior Applies the damage one attacker deals to one target battler and
 *   returns the amount. It first marks the target status byte 0x11 (the
 *   D_80145FB0 record byte below three, the D_801EB72C enemy byte at or above
 *   three), then resolves the amount through resolveBattlerDamageAmount with
 *   the 0xFFFF kind mask. While the target flag word carries 0x100 the amount
 *   becomes the target current HP halfword (D_80145F18 for a party target; for
 *   an enemy target the D_801EB6C4 halfword, but only while that halfword is at
 *   most 0x7FFF), the 0x1E event row is queued
 *   with 0x801EB075, 0x801483C3 is raised and 0x100 is cleared. A positive
 *   amount subtracts from the HP halfword; a party target whose shortfall
 *   stays under the D_80145F1C threshold byte and whose D_80145FB8 word holds
 *   neither bit 1 nor 2 also raises bit 4 of D_80145FB4, and the HP halfword is
 *   zeroed either way, while an enemy halfword simply zeroes. A non-positive
 *   amount adds to the HP halfword clamped at the target maximum (D_80145F20
 *   for a party target, D_801EB6D0 for an enemy) with status bit 4 raised on
 *   the clamp. A party attacker whose D_80145FCC byte reads 5 and whose
 *   D_80145F12 kind byte is 0x4A, 0x4C or 0x4F then rolls func_800A3DF8 and,
 *   on a zero result, applies the matching 0x40, 0x8 or 0x20 mask to the
 *   target through func_800A31E0.
 * @status exact
 * @match 100.00
 * @residual none
 */
s16 applyResolvedDamageToTarget(u8 attacker, u8 target) {
  u32 offset;
  u32 enemy_init;
  s32 amount;
  u16 hp;
  u16 limit;
  u32 flags;
  u8 threshold;

  if (target < 3u) {
    D_80145FB0[target * 0x140] = 0x11;
  } else {
    enemy_init = (target - 3) * 0x118;
    D_801EB72C[enemy_init] = 0x11;
  }

  amount = (s16)resolveBattlerDamageAmount(attacker, target, 0xffffu);

  if (target < 3u) {
    offset = target * 0x140;
    if ((D_80145FB4[target].flags_00 & 0x100u) != 0u) {
      amount = *(u16 *)(D_80145F18 + offset);
      func_801DE560(1u, 0u, 0u, 0x1eu, (u32)D_801EB075);
      D_801483C3 = 1;
      D_80145FB4[target].flags_00 &= ~0x100u;
    }
    if (amount > 0) {
      hp = *(u16 *)(D_80145F18 + offset);
      if (amount < (hp & 0xFFFF)) {
        *(u16 *)(D_80145F18 + offset) = hp - amount;
      } else {
        threshold = D_80145F1C[offset];
        if ((amount - (hp & 0xFFFF)) < threshold) {
          flags = D_80145FB8[target].flags_00;
          if ((flags & 1u) == 0u) {
            if ((flags & 2u) == 0u) {
              D_80145FB4[target].flags_00 |= 4u;
            }
          }
        }
        *(u16 *)(D_80145F18 + target * 0x140) = 0;
      }
    } else {
      hp = *(u16 *)(D_80145F18 + offset);
      limit = *(u16 *)(D_80145F20 + offset);
      if ((hp - amount) < limit) {
        *(u16 *)(D_80145F18 + offset) = hp - amount;
      } else {
        *(u16 *)(D_80145F18 + offset) = limit;
        D_80145FB0[offset] |= 4;
      }
    }
  } else {
    u16 ehp;
    u32 doff;

    offset = (target - 3) * 0x118;
    ehp = *(u16 *)(D_801EB6C4 + offset);
    if (ehp == 0xffffu) {
      goto damage_applied;
    }
    if ((*(u32 *)(D_801EB730 + offset) & 0x100u) != 0u) {
      if (!(0x7fffu < ehp)) {
        amount = ehp;
        func_801DE560(1u, 0u, 0u, 0x1eu, (u32)D_801EB075);
        D_801483C3 = 1;
        *(u32 *)(D_801EB730 + offset) &= ~0x100u;
      }
    }
    if (amount > 0) {
      doff = (target - 3) * 0x118;
      ehp = *(u16 *)(D_801EB6C4 + doff);
      if (amount < ehp) {
        *(u16 *)(D_801EB6C4 + doff) = ehp - amount;
      } else {
        *(u16 *)(D_801EB6C4 + doff) = 0;
      }
    } else {
      u16 hlim;

      doff = (target - 3) * 0x118;
      ehp = *(u16 *)(D_801EB6C4 + doff);
      hlim = *(u16 *)(D_801EB6D0 + doff);
      if ((ehp - amount) < hlim) {
        *(u16 *)(D_801EB6C4 + doff) = ehp - amount;
      } else {
        *(u16 *)(D_801EB6C4 + doff) = hlim;
        D_801EB72C[doff] |= 4;
      }
    }
  }

damage_applied:
  if (attacker < 3u) {
    if ((D_80145FCC[attacker * 0x140] == 5u) && (amount != 0)) {
      if (D_80145F12[attacker * 0x140] == 0x4au) {
        if (func_800A3DF8(attacker, target) == 0u) {
          func_800A31E0(target, 0x40u);
        }
      }
      if (D_80145F12[attacker * 0x140] == 0x4cu) {
        if (func_800A3DF8(attacker, target) == 0u) {
          func_800A31E0(target, 0x8u);
        }
      }
      if (D_80145F12[attacker * 0x140] == 0x4fu) {
        if (func_800A3DF8(attacker, target) == 0u) {
          func_800A31E0(target, 0x20u);
        }
      }
    }
  }

  return (s16)amount;
}
