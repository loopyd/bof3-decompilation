#include "internal.h"

/* @behavior initializes the primary battle selection-grid scratch band from the
 * current active message slot and the saved group/page/cursor bytes.
 * @source 0x80097EC0
 */
void func_80097EC0(void) {
  volatile u8* state;
  volatile u8* message_slot;
  volatile u8* active_selection_slot;
  u32          source_slot_offset;
  u32          active_selection_flags;

  state = (volatile u8*)0x80148570u;
  state[0] = 1u;
  state[1] = 8u;
  message_slot = *(volatile u8**)0x801ebf08u;
  state[2] = 2u;
  state[3] = 2u;

  source_slot_offset = (u32)message_slot[5] * 3u;
  state[0xa] = message_slot[5];
  state[0xb] = *(volatile u8*)(0x801454f4u + source_slot_offset);
  *(volatile u16*)(state + 0x10) =
      *(volatile u8*)(0x801454f5u + source_slot_offset);
  state[0xd] = 0xffu;
  state[8] = 2u;
  state[9] = 0u;
  state[0xc] = *(volatile u8*)(0x801454f6u + source_slot_offset);

  active_selection_slot = *(volatile u8**)0x801eb4d8u;
  active_selection_flags = *(volatile u32*)(active_selection_slot + 0x10);
  *(volatile u16*)(state + 0x14) = (u16)(active_selection_flags & 2u);
  if ((active_selection_flags & 0x20002u) != 2u) {
    *(volatile u16*)(state + 0x14) = 0u;
  }

  *(volatile u16*)(state + 4) = 0x140u;
  *(volatile u16*)(state + 6) = 0x3fu;
  *(volatile u8*)0x801485dcu = 0u;
  *(volatile u8*)0x801485ddu = 8u;
  *(volatile u8*)0x801485deu = 0u;
  *(volatile u8*)0x80148656u = message_slot[5];
}
