#include "bof3/battle/battle03_internal.h"

/* @behavior finds the first free event slot in the eight-entry event queue and
 * populates it with the caller's parameters.
 * @source 0x801DE560
 * @status partial
 * @match 20.83
 * @residual non-exact live audit: 10/43 instructions; 172 original bytes versus 192 current.
 */
void func_801DE560(u8 arg0, u8 arg1, u8 arg2, u8 arg3, u32 arg4) {
  u8  index;
  u8  value;
  u32 offset;

  index = 2u;
  while (index < 8u) {
    offset = (u32)index * 0xcu;
    value = BATTLE_EVENT_SLOT_FLAG(index);
    if (value == 0u) {
      BATTLE_EVENT_SLOT_FLAG(index) = value | 1u;
      BATTLE_EVENT_SLOT_A(index) = arg0;
      BATTLE_EVENT_SLOT_B(index) = arg1;
      BATTLE_EVENT_SLOT_C(index) = arg2;
      BATTLE_EVENT_SLOT_HALF(index) = arg3;
      BATTLE_EVENT_SLOT_WORD(index) = arg4;
      BATTLE_EVENT_SLOT_BYTE(index) = 0u;
      return;
    }
    index += 1u;
  }
}
