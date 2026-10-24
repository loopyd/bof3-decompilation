#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x8009D198
 * @behavior Applies the pending damage pair of the active battle selection
 *   record pointed to by D_8014639C.record to the battler named by the index byte
 *   D_80146394 (below 3 the 0x140-stride local-work record based at
 *   0x80145F18, otherwise the 0x118-stride enemy-work record based at
 *   0x801EB6C4), then dispatches the current battle mode's action handler.
 *   Marks the target status byte with 0x31, consumes the mode-scaled counter
 *   of D_80146374, clears the pair when either record flag word holds
 *   0x10000, then clamps and consumes the record halfwords +0x04 and +0x06.
 * @status partial
 * @match 70.78
 * @residual 407/575 instructions, 2244/2300 bytes. Remaining differences
 *   include party-field address formation, register allocation and branch
 *   scheduling. The two clamp/read phases retain separate operation scopes;
 *   the dispatch selector has its own halfword lifetime.
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
    u16 action;
    action = *(u16*)(D_80146380 + 2);
    D_800B471C[D_800B470C[action >> 8][action & 0xFF]]();
  }

  mode = *(u8*)&D_80146394;
  if (mode < 3) {
    flags = D_80145FB0[mode].flags;
  } else {
    flags = D_801EB72C[mode - 3].flags;
  }
  if ((flags & 0x10000u) != 0u) {
    rec = D_8014639C.record;
    rec[2] = 0;
    rec = D_8014639C.record;
    rec[3] = 0;
  }

  do {
    rec = D_8014639C.record;
    if (rec[2] >= 10000) {
      rec[2] = 9999;
    }
    rec = D_8014639C.record;
    if (rec[2] < -9999) {
      rec[2] = -9999;
    }

    rec = D_8014639C.record;
    amount = rec[2];
  } while (0);
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

  do {
    rec = D_8014639C.record;
    if (rec[3] >= 10000) {
      rec[3] = 9999;
    }
    rec = D_8014639C.record;
    if (rec[3] < -9999) {
      rec[3] = -9999;
    }

    rec = D_8014639C.record;
    amount = rec[3];
  } while (0);
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
