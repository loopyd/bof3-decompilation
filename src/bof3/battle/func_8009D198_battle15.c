#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x8009D198
 * @behavior Applies the pending damage pair of the active battle selection
 *   record pointed to by D_801463A0 to the battler named by the index byte
 *   D_80146394 (below 3 the 0x140-stride local-work record based at
 *   0x80145F18, otherwise the 0x118-stride enemy-work record based at
 *   0x801EB6C4), then dispatches the current battle mode's action handler.
 *   Marks the target status byte with 0x31, consumes the mode-scaled counter
 *   of D_80146374, clears the pair when either record flag word holds
 *   0x10000, then clamps and consumes the record halfwords +0x04 and +0x06.
 * @status partial
 * @match 68.87
 * @residual 396/575 instructions, 2244/2300 bytes (56 short; the candidate
 *   body is 561 instructions against the original's 575). Four clean-C shapes
 *   carry this reading. (1) The record halfwords, the length byte and the
 *   limit/reset halfwords are reached through the per-address symbols the
 *   original names (D_80145F1A/D_80145F1C/D_80145F22 and
 *   D_801EB6C6/D_801EB6D0/D_801EB6D2), so each of those accesses emits the
 *   original's `lui $at,%hi(SYM); addu $at,$at,index; op %lo(SYM)($at)`
 *   sequence instead of one pooled base register. (2) Both enemy damage blocks
 *   keep a single shared `value - amount` store: the original reaches one store
 *   from both comparison arms (0x8009D730 for the halfword +0x04 block and
 *   0x8009D9D8 for the halfword +0x06 block), so the zero/limit/reset arms are
 *   written as early exits and the common store follows the branch, which is
 *   what lets GCC cross-jump them. The labelled early exit is an ordinary
 *   matched-label shape, not a matching aid. (3) The `(index - 3) * 0x118`
 *   record byte offset is materialised once per branch in eoff/eoff2/eoff3
 *   instead of being respelled at every access, as the original does (one
 *   offset register feeds the flag word, the counter byte and the +0x19 clear
 *   of the same branch). (4) The four enemy status read-modify-writes are
 *   spelled through the header's volatile `D_80146394` index instead of the
 *   stripped `u8 *state` view, because the original re-reads the index byte
 *   and re-derives `(index - 3) * 0x118` before each of those statements
 *   (`lbu v1,0(t1)` at original.s 367, 383, 540 and 556). Measured on its own
 *   this moved the reading from 392/575 at 2156 to 396/575 at 2244.
 *   What remains is almost all on the party side. The party-side +0x02/+0x0A
 *   halfword accesses still pool a materialised 0x80145F1A base (`lui/addiu`
 *   + `lhu/sh 0(reg)`) where the original re-emits
 *   `lui $at,%hi(SYM); addu $at,$at,offset; op %lo(SYM)($at)` per access: the
 *   eleven hunks outside the two enemy arm spans net -18 (the four non-zero
 *   ones are -4, -1, -3 and -10 lines), so the party side and the block
 *   preambles carry the whole shortfall (561 against 575 is -14 once the enemy
 *   arms' +4 is netted). The two enemy damage blocks are net longer, not
 *   shorter: +2 each, 71 and 74 instructions against the original's 69 and 72
 *   when both spans run from the `(index - 3) * 0x118` stride setup to the end
 *   of the block. Shape (4) is what pays there: each of the four enemy status
 *   statements spends a fresh `lui v1,0x8014; lbu v1,25492(v1)` where the
 *   original re-reads `lbu v1,0(t1)` through the per-damage-block hoist it
 *   keeps above the arm dispatch (`lui t1,0x8014; addiu t1,t1,25492` at
 *   original.s 223-224 and 424-425, mirrored by current.s 224 and 419), so the
 *   +4 is the per-statement price alone. From the battle-mode byte load
 *   onward the original's allocator also picks $v1 where this candidate picks
 *   $a0. Consumed evidence, in order:
 *   non-volatile views for the over-declared volatile globals
 *   D_80146394/D_80146374/D_801463C0 (54.26 -> 55.30), a counter pointer local
 *   for D_801EC312 (-> 55.30), split access views for the D_80145FC9 counter
 *   cell (-> 56.87), a hoisted enemy record index (equal score, larger size), a
 *   hoisted 0x140 byte offset (55.30) and a separate second index local
 *   (55.13); a `volatile u8*` state view (350/575 but 2836 bytes and a 0x68
 *   frame against the original's 0x28); the per-field symbol conversion on its
 *   own (327/575, 2200); the merged tail store in the +0x06 block on its own
 *   (329/575, 2244); the +0x19 clear spelled through D_80145FC9/D_801EB745
 *   (326/575, 2236) with or without the +0x04 block merge (343/575, 2280); the
 *   +0x19 offset hoist on its own (374/575, 2272); and, on top of it, a hoisted
 *   0x140 party offset in both damage blocks (390/575, 2092) and direct
 *   volatile reads of D_80146394 in both blocks (402/575, 2704, frame -0x48) or
 *   in the second block only (396/575, 2384, frame -0x38): higher match counts
 *   but a prologue and size drift the original does not have, so they were not
 *   retained. The scoped shape (4) above is a different lever from those two:
 *   it changes only the four enemy status statements, keeps the 0x28 frame and
 *   moves the size toward the original, and applying it to one block alone
 *   measured 388/575 at 2200 (+0x04 block) and 384/575 at 2200 (+0x06 block),
 *   so the two blocks only pay off together. Repair round 4 measured two more
 *   shapes: per-party-arm `goto` exits to a label at the end of each damage
 *   block left the emitted body byte-identical (each arm's store is already the
 *   last statement of its arm, so the exit is a no-op; 396/575 at 2244), and a
 *   scoped `volatile u8 *` base assigned once per enemy block for the two
 *   status statements of shape (4) measured 396/575 at 2228, with the block
 *   stride also read through that base 396/575 at 2244. Equal matches in both
 *   cases, and 2228 bytes moves away from the original's 2300, so neither was
 *   retained. Withheld, untried and not exhausted: the selector compiler
 *   profile/flag search and the permuter.
 *   Callers: this target holds no reference to 0x8009D198, but overlay
 *   BIN/BATTLE/BATTLE.EMI#3 imports the address (it is listed in that overlay's
 *   undefined_funcs_auto.txt) and calls it twice, at 0x801DFE64 in
 *   func_801DFCB4 and at 0x801E3D68 in func_801E3C14. Both sites guard the call
 *   with the battle-global byte 0x80146375 (`beqz` / `slti 6` / `beqz` /
 *   `slti 4` / `bnez`), so it is taken only when that byte is 4 or 5; both set
 *   no argument register and read no result, which corroborates the void
 *   prototype and this body's D_80146375 == 4 dispatch. In both callers the
 *   sibling `== 1` arm stores its result to the 0x801463A0 record halfword
 *   +0x04, the halfword this body clamps. Overlay 03 and overlay 15 are not
 *   proven resident together, so this stays caller evidence only: no shared
 *   symbol, header or map row is added and the address-anchored name stays.
 *   No aids: no register pins, clobbers, barriers or empty asm.
 */
void func_8009D198(void) {
  u32 index;
  u32 mode;
  u16* counter;
  s16* rec;
  u8* state;
  u16 value;
  u16 reset;
  u16 limit;
  u8 byte_limit;
  s16 amount;
  u32 flags;

  index = *(u8*)&D_80146394;
  if (index < 3) {
    D_80145FB0[index].status |= 0x31;
  } else {
    D_801EB72C[index - 3].status |= 0x31;
  }

  if ((D_80146375 != 4u) || (*(u16*)&D_801463C0 != 0xa3u)) {
    index = *(u8*)&D_80146374;
    if (index < 3u) {
      if ((D_80145FB8[index].flags_00 & 0x80u) != 0u) {
        counter = &D_801EC312;
        *counter = (u16)(*counter * (D_80145FC9[index * 0x140] + 1));
        ((u8*)D_80145FB0)[index * 0x140 + 0x19] = 0u;
        D_80145FB8[D_80146374].flags_00 &= 0xffffff7fu;
      }
    } else {
      u32 eoff;
      if ((D_801EB734[index - 3].word_00 & 0x80u) != 0u) {
        eoff = (index - 3) * 0x118u;
        counter = &D_801EC312;
        *counter = (u16)(*counter * (D_801EB745[eoff] + 1));
        ((u8*)D_801EB72C)[eoff + 0x19] = 0u;
        D_801EB734[D_80146374 - 3].word_00 &= 0xffffff7fu;
      }
    }
  }

  if (D_80146375 == 4u) {
    D_800B471C[D_800B44F8[*(u16*)&D_801463C0]]();
  } else {
    value = *(u16*)(D_80146380 + 2);
    D_800B471C[D_800B470C[value >> 8][value & 0xFF]]();
  }

  mode = *(u8*)&D_80146394;
  if (mode < 3) {
    flags = D_80145FB0[mode].flags;
  } else {
    flags = D_801EB72C[mode - 3].flags;
  }
  if ((flags & 0x10000u) != 0u) {
    rec = D_801463A0;
    rec[2] = 0;
    rec = D_801463A0;
    rec[3] = 0;
  }

  rec = D_801463A0;
  if (rec[2] >= 10000) {
    rec[2] = 9999;
  }
  rec = D_801463A0;
  if (rec[2] < -9999) {
    rec[2] = -9999;
  }

  rec = D_801463A0;
  amount = rec[2];
  if (amount != 0) {
    state = (u8*)&D_80146394;
    if (*state < 3) {
      if (amount > 0) {
        value = *(u16*)(D_80145F18 + *state * 0x140);
        if (amount < (value & 0xFFFF)) {
          *(u16*)(D_80145F18 + *state * 0x140) = value - amount;
        } else {
          byte_limit = *(u8*)(D_80145F1C + *state * 0x140);
          if ((amount - (value & 0xFFFF)) < byte_limit) {
            if ((D_80145FB8[*state].flags_00 & 1u) == 0u) {
              if ((D_80145FB8[*state].flags_00 & 2u) == 0u) {
                D_80145FB0[*state].flags |= 4;
              }
            }
          }
          *(u16*)(D_80145F18 + *state * 0x140) = 0;
        }
      } else {
        value = *(u16*)(D_80145F18 + *state * 0x140);
        limit = *(u16*)(D_80145F20 + *state * 0x140);
        if ((value - amount) < limit) {
          *(u16*)(D_80145F18 + *state * 0x140) = value - amount;
        } else {
          *(u16*)(D_80145F18 + *state * 0x140) = limit;
          D_80145FB0[*state].status |= 5;
          D_80145FB0[*state].status &= ~0x10;
        }
      }
    } else {
      u32 eoff2;
      eoff2 = (*state - 3) * 0x118u;
      value = *(u16*)(D_801EB6C4 + eoff2);
      if ((value & 0xFFFF) != 0xFFFF) {
        if (amount > 0) {
          if (amount >= (value & 0xFFFF)) {
            *(u16*)(D_801EB6C4 + eoff2) = 0;
            goto first_damage_store_done;
          }
        } else {
          limit = *(u16*)(D_801EB6D0 + eoff2);
          if (((value & 0xFFFF) - amount) >= limit) {
            *(u16*)(D_801EB6C4 + eoff2) = limit;
            D_801EB72C[D_80146394 - 3].status |= 5;
            D_801EB72C[D_80146394 - 3].status &= ~0x10;
            goto first_damage_store_done;
          }
        }
        *(u16*)(D_801EB6C4 + eoff2) = value - amount;
      }
    first_damage_store_done:
      ;
    }
  }

  rec = D_801463A0;
  if (rec[3] >= 10000) {
    rec[3] = 9999;
  }
  rec = D_801463A0;
  if (rec[3] < -9999) {
    rec[3] = -9999;
  }

  rec = D_801463A0;
  amount = rec[3];
  if (amount != 0) {
    state = (u8*)&D_80146394;
    if (*state < 3) {
      if (amount > 0) {
        value = *(u16*)(D_80145F1A + *state * 0x140);
        if (amount < value) {
          *(u16*)(D_80145F1A + *state * 0x140) = value - amount;
        } else {
          *(u16*)(D_80145F1A + *state * 0x140) = 0;
        }
      } else {
        value = *(u16*)(D_80145F1A + *state * 0x140);
        limit = *(u16*)(D_80145F22 + *state * 0x140);
        if ((value - amount) < limit) {
          *(u16*)(D_80145F1A + *state * 0x140) = value - amount;
        } else {
          *(u16*)(D_80145F1A + *state * 0x140) = limit;
          D_80145FB0[*state].status |= 0xA;
          D_80145FB0[*state].status &= ~0x20;
        }
      }
    } else {
      u32 eoff3;
      eoff3 = (*state - 3) * 0x118u;
      value = *(u16*)(D_801EB6C6 + eoff3);
      if ((value & 0xFFFF) != 0xFFFF) {
        if (amount > 0) {
          if (amount >= (value & 0xFFFF)) {
            *(u16*)(D_801EB6C6 + eoff3) = 0;
            goto second_damage_store_done;
          }
        } else {
          limit = *(u16*)(D_801EB6D0 + eoff3);
          if (((value & 0xFFFF) - amount) >= limit) {
            reset = *(u16*)(D_801EB6D2 + eoff3);
            *(u16*)(D_801EB6C6 + eoff3) = reset;
            D_801EB72C[D_80146394 - 3].status |= 0xA;
            D_801EB72C[D_80146394 - 3].status &= ~0x20;
            goto second_damage_store_done;
          }
        }
        *(u16*)(D_801EB6C6 + eoff3) = value - amount;
      }
    second_damage_store_done:
      ;
    }
  }
}
