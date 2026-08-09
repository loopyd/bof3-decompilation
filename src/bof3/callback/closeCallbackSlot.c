#include "bof3/core/slus_internal.h"

extern u16 D_80143B40;
extern s32 D_80143B48;

/* @behavior clears one callback slot and closes its thread inside the scheduler
 * critical section.
 * @source 0x8014B900
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void closeCallbackSlot(s32 slot_index) {
  s32 slot_offset;

  slot_offset = slot_index << 7;
  *(u16*)((u8*)&D_80143B40 + slot_offset) = 0;
  EnterCriticalSection();
  CloseTh(*(s32*)((u8*)&D_80143B48 + slot_offset));
  ExitCriticalSection();
}
