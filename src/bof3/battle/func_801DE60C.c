#include "bof3/battle/battle03_internal.h"

/* @behavior overwrites one event-queue slot directly from the caller's parameters
 * and marks the slot active.
 * @source 0x801DE60C
 * @status partial
 * @match 20.51
 * @residual non-exact live audit: 8/33 instructions; 132 original bytes versus 156 current.
 */
void func_801DE60C(u32 arg0, u8 arg1, u8 arg2, u8 arg3, u8 arg4, u32 arg5) {
  u8 flag;

  arg0 &= 0xffu;
  flag = BATTLE_EVENT_SLOT_FLAG(arg0);
  BATTLE_EVENT_SLOT_A(arg0) = arg1;
  BATTLE_EVENT_SLOT_B(arg0) = arg2;
  BATTLE_EVENT_SLOT_C(arg0) = arg3;
  BATTLE_EVENT_SLOT_BYTE(arg0) = 0u;
  BATTLE_EVENT_SLOT_FLAG(arg0) = flag | 1u;
  BATTLE_EVENT_SLOT_HALF(arg0) = arg4;
  BATTLE_EVENT_SLOT_WORD(arg0) = arg5;
}
