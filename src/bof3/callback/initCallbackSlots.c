#include "bof3/core/slus_internal.h"

extern volatile GameCallbackSlot D_80143B40[4];

/* @behavior clears each of the four EXE callback slots and gives it a private
 * thread context 0xC0 bytes above the base word read at fixed address 0x110, a
 * matching 0x200-byte stack top below 0x80200000, and the fixed word 0x40000404
 * at offset 0x94 of that context.
 * @source 0x8014B6B4
 * @status exact
 * @match 100.00
 * @residual none
 */
void initCallbackSlots(void) {
  u32* context_anchor;
  s32  i;
  u32  context_base;
  u32  context;
  u32  context_offset;

  context_anchor = (u32*)0x100;

  for (i = 0; i < 4; i++) {
    D_80143B40[i].state = GAME_CALLBACK_SLOT_STATE_EMPTY;
    context_base = context_anchor[4];
    context_offset = i * 0xC0 + 0xC0;
    D_80143B40[i].unk_0c = context_base + context_offset;
    context = D_80143B40[i].unk_0c;
    D_80143B40[i].open_arg = (i + 1) * 0x200 + 0x801FF400;
    *(u32*)(context + 0x94) = 0x40000404;
  }
}
