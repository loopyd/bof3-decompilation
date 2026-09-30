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
 * @match 56.52
 * @residual 325/575 instructions, 2172/2300 bytes; first difference at
 *   +0x00a4 (instruction 41): the original loads the battle-mode index byte
 *   D_80146374 into $v1 while this candidate's allocator selects $a0, and the
 *   two residuals behind it are register naming inside otherwise identical
 *   blocks plus the address form of the damage blocks, where the original
 *   recomputes `lui $at,%hi; addu $at,$at,REG; <op> %lo($at)` per access and
 *   this candidate materializes one base register instead. Clean-C rungs
 *   measured live with bin/shape-sweep: non-volatile views for the
 *   over-declared volatile globals D_80146394/D_80146374/D_801463C0 (54.26 ->
 *   55.30, removes a spurious `andi` and a duplicate load), counter pointer
 *   local for D_801EC312 (-> 55.30), split access views for the D_80145FC9
 *   counter cell (-> 56.87) and a hoisted enemy record index (equal score,
 *   larger size; reverted), a hoisted 0x140 byte offset (regressed to 55.30,
 *   reverted) and a separate second index local (regressed to 55.13,
 *   reverted). Root cause of most of the residue: battle15_internal.h
 *   declares D_80146394/D_80146374/D_801463C0 volatile, while the
 *   independently exact battle03 sibling func_801D4850 shows the same RAM
 *   globals non-volatile; the declaration fix would affect many other
 *   battle15 lifts and is left to the parent. Next untried: that declaration
 *   correction plus the opt-in profile/permuter rungs, which this mission was
 *   not authorized to run. No aids: no register pins, clobbers, barriers or
 *   empty asm.
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
      if ((D_801EB734[index - 3].word_00 & 0x80u) != 0u) {
        counter = &D_801EC312;
        *counter = (u16)(*counter * (D_801EB745[(index - 3) * 0x118] + 1));
        ((u8*)D_801EB72C)[(index - 3) * 0x118 + 0x19] = 0u;
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
          byte_limit = *(u8*)(D_80145F18 + *state * 0x140 + 4);
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
        limit = *(u16*)(D_80145F18 + *state * 0x140 + 8);
        if ((value - amount) < limit) {
          *(u16*)(D_80145F18 + *state * 0x140) = value - amount;
        } else {
          *(u16*)(D_80145F18 + *state * 0x140) = limit;
          D_80145FB0[*state].status |= 5;
          D_80145FB0[*state].status &= ~0x10;
        }
      }
    } else {
      value = *(u16*)(D_801EB6C4 + (*state - 3) * 0x118);
      if ((value & 0xFFFF) != 0xFFFF) {
        if (amount > 0) {
          if (amount < (value & 0xFFFF)) {
            *(u16*)(D_801EB6C4 + (*state - 3) * 0x118) = value - amount;
          } else {
            *(u16*)(D_801EB6C4 + (*state - 3) * 0x118) = 0;
          }
        } else {
          limit = *(u16*)(D_801EB6C4 + (*state - 3) * 0x118 + 0xC);
          if (((value & 0xFFFF) - amount) < limit) {
            *(u16*)(D_801EB6C4 + (*state - 3) * 0x118) = value - amount;
          } else {
            *(u16*)(D_801EB6C4 + (*state - 3) * 0x118) = limit;
            D_801EB72C[*state - 3].status |= 5;
            D_801EB72C[*state - 3].status &= ~0x10;
          }
        }
      }
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
        value = *(u16*)(D_80145F18 + *state * 0x140 + 2);
        if (amount < value) {
          *(u16*)(D_80145F18 + *state * 0x140 + 2) = value - amount;
        } else {
          *(u16*)(D_80145F18 + *state * 0x140 + 2) = 0;
        }
      } else {
        value = *(u16*)(D_80145F18 + *state * 0x140 + 2);
        limit = *(u16*)(D_80145F18 + *state * 0x140 + 0xA);
        if ((value - amount) < limit) {
          *(u16*)(D_80145F18 + *state * 0x140 + 2) = value - amount;
        } else {
          *(u16*)(D_80145F18 + *state * 0x140 + 2) = limit;
          D_80145FB0[*state].status |= 0xA;
          D_80145FB0[*state].status &= ~0x20;
        }
      }
    } else {
      value = *(u16*)(D_801EB6C4 + (*state - 3) * 0x118 + 2);
      if ((value & 0xFFFF) != 0xFFFF) {
        if (amount > 0) {
          if (amount < (value & 0xFFFF)) {
            *(u16*)(D_801EB6C4 + (*state - 3) * 0x118 + 2) =
                value - amount;
          } else {
            *(u16*)(D_801EB6C4 + (*state - 3) * 0x118 + 2) = 0;
          }
        } else {
          limit = *(u16*)(D_801EB6C4 + (*state - 3) * 0x118 + 0xC);
          if (((value & 0xFFFF) - amount) < limit) {
            *(u16*)(D_801EB6C4 + (*state - 3) * 0x118 + 2) =
                value - amount;
          } else {
            reset = *(u16*)(D_801EB6C4 + (*state - 3) * 0x118 + 0xE);
            *(u16*)(D_801EB6C4 + (*state - 3) * 0x118 + 2) = reset;
            D_801EB72C[*state - 3].status |= 0xA;
            D_801EB72C[*state - 3].status &= ~0x20;
          }
        }
      }
    }
  }
}
