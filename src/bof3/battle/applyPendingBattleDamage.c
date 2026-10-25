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
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyPendingBattleDamage(void) {
  u32 index;
  u32 mode;
  u16* counter;
  u8* state;
  u16 value;
  u16 first_current_value;
  u8 byte_limit;
  s16 amount;

  index = *(u8*)&D_80146394;
  if (index < 3) {
    D_80145FB0[index].status |= 0x31;
  } else {
    D_801EB72C[index - 3].status |= 0x31;
  }

  if ((D_80146375 != 4u) || (*(u16*)&D_801463C0 != 0xa3u)) {
    u32 pending_index;
    pending_index = *(u8*)&D_80146374;
    if (pending_index < 3u) {
      if ((D_80145FB8[pending_index].flags_00 & 0x80u) != 0u) {
        counter = &D_801EC312;
        *counter = (u16)(*counter * (D_80145FC9[pending_index * 0x140] + 1));
        ((u8*)D_80145FB0)[pending_index * 0x140 + 0x19] = 0u;
        D_80145FB8[D_80146374].flags_00 &= 0xffffff7fu;
      }
    } else {
      u32 eoff;
      if ((D_801EB734[pending_index - 3].word_00 & 0x80u) != 0u) {
        eoff = (pending_index - 3) * 0x118u;
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
    if ((D_80145FB0[mode].flags & 0x10000u) == 0u) {
      goto flags_cleared;
    }
  } else {
    if ((D_801EB72C[mode - 3].flags & 0x10000u) == 0u) {
      goto flags_cleared;
    }
  }
  D_8014639C.record[2] = 0;
  D_8014639C.record[3] = 0;
flags_cleared:
  ;

  {
    if (D_8014639C.record[2] >= 10000) {
      D_8014639C.record[2] = 9999;
    }
    if (D_8014639C.record[2] < -9999) {
      D_8014639C.record[2] = -9999;
    }

    amount = D_8014639C.record[2];
  }
  if (amount != 0) {
    state = (u8*)&D_80146394;
    if (*state < 3) {
      u32 party_slot1 = *state;
      u32 party_four1 = party_slot1 << 2;
      if (amount > 0) {
        u16 party_positive1;
        party_positive1 = *(u16*)(D_80145F18 + ((party_four1 + party_slot1) << 6));
        if (amount < (party_positive1 & 0xFFFF)) {
          *(u16*)(D_80145F18 + ((party_four1 + party_slot1) << 6)) = party_positive1 - amount;
        } else {
          byte_limit = *(u8*)(D_80145F1C + *state * 0x140);
          if ((amount - (party_positive1 & 0xFFFF)) < byte_limit) {
            if ((D_80145FB8[*state].flags_00 & 1u) == 0u) {
              if ((D_80145FB8[*state].flags_00 & 2u) == 0u) {
                D_80145FB0[*state].flags |= 4;
              }
            }
          }
          *(u16*)(D_80145F18 + D_80146394 * 0x140) = 0;
        }
      } else {
        u16 party_limit1;
        first_current_value = *(u16*)(D_80145F18 + ((party_four1 + party_slot1) << 6));
        party_limit1 = *(u16*)(D_80145F20 + ((party_four1 + party_slot1) << 6));
        if ((first_current_value - amount) < party_limit1) {
          *(u16*)(D_80145F18 + ((party_four1 + party_slot1) << 6)) = first_current_value - amount;
        } else {
          *(u16*)(D_80145F18 + ((party_four1 + party_slot1) << 6)) = party_limit1;
          D_80145FB0[*(volatile u8*)state].status |= 5;
          {
            u32 status_slot = *(volatile u8*)state;
            amount = ~D_80145FB0[status_slot].status;
            D_80145FB0[status_slot].status = ~(amount | 0x10);
          }
        }
      }
    } else {
      u16 enemy_limit1;
      u32 eoff2;
      eoff2 = (*state - 3) * 0x118u;
      first_current_value = *(u16*)(D_801EB6C4 + eoff2);
      if ((first_current_value & 0xFFFF) == 0xFFFF) {
        goto first_damage_store_done;
      }
      if (amount > 0) {
        if (amount >= (first_current_value & 0xFFFF)) {
          *(u16*)(D_801EB6C4 + eoff2) = 0;
          goto first_damage_store_done;
        }
      } else {
        enemy_limit1 = *(u16*)(D_801EB6D0 + eoff2);
        if (((first_current_value & 0xFFFF) - amount) >= enemy_limit1) {
          goto capped_damage1;
        }
      }
      *(u16*)(D_801EB6C4 + eoff2) = first_current_value - amount;
      goto first_damage_store_done;
    capped_damage1:
      {
        *(u16*)(D_801EB6C4 + eoff2) = enemy_limit1;
        D_801EB72C[*(volatile u8*)state - 3].status |= 5;
        D_801EB72C[*(volatile u8*)state - 3].status &= ~0x10;
      }
    first_damage_store_done:
      ;
    }
  }

  {
    if (D_8014639C.record[3] >= 10000) {
      D_8014639C.record[3] = 9999;
    }
    if (D_8014639C.record[3] < -9999) {
      D_8014639C.record[3] = -9999;
    }

    amount = D_8014639C.record[3];
  }
  if (amount != 0) {
    state = (u8*)&D_80146394;
    if (*state < 3) {
      u32 party_slot2 = *state;
      u32 party_four2 = party_slot2 << 2;
      if (amount > 0) {
        u16 party_positive2;
        party_positive2 = *(u16*)(D_80145F1A + ((party_four2 + party_slot2) << 6));
        if (amount < party_positive2) {
          *(u16*)(D_80145F1A + ((party_four2 + party_slot2) << 6)) = party_positive2 - amount;
        } else {
          *(u16*)(D_80145F1A + *state * 0x140) = 0;
        }
      } else {
        u16 party_limit2;
        u16 party_negative2;
        party_negative2 = *(u16*)(D_80145F1A + ((party_four2 + party_slot2) << 6));
        party_limit2 = *(u16*)(D_80145F22 + ((party_four2 + party_slot2) << 6));
        if ((party_negative2 - amount) < party_limit2) {
          *(u16*)(D_80145F1A + ((party_four2 + party_slot2) << 6)) = party_negative2 - amount;
        } else {
          *(u16*)(D_80145F1A + ((party_four2 + party_slot2) << 6)) = party_limit2;
          D_80145FB0[*(volatile u8*)state].status |= 0xA;
          D_80145FB0[*(volatile u8*)state].status &= ~0x20;
        }
      }
    } else {
      u16 enemy_reset2;
      u16 enemy_limit2;
      index = (*state - 3) * 0x118u;
      value = *(u16*)(D_801EB6C6 + index);
      if ((value & 0xFFFF) == 0xFFFF) {
        goto second_damage_store_done;
      }
      if (amount > 0) {
        if (amount >= (value & 0xFFFF)) {
          *(u16*)(D_801EB6C6 + index) = 0;
          goto second_damage_store_done;
        }
      } else {
        enemy_limit2 = *(u16*)(D_801EB6D0 + index);
        if (((value & 0xFFFF) - amount) >= enemy_limit2) {
          goto capped_damage2;
        }
      }
      *(u16*)(D_801EB6C6 + index) = value - amount;
      goto second_damage_store_done;
    capped_damage2:
      {
        enemy_reset2 = *(u16*)(D_801EB6D2 + index);
        *(u16*)(D_801EB6C6 + index) = enemy_reset2;
        D_801EB72C[*(volatile u8*)state - 3].status |= 0xA;
        D_801EB72C[*(volatile u8*)state - 3].status &= ~0x20;
      }
    second_damage_store_done:
      ;
    }
  }
}
