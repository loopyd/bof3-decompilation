#include "internal.h"

extern u16 DAT_80143b40;
extern s32 DAT_80143b48;

/* does: clears one callback slot and closes its thread inside the scheduler
 * critical section.
 * @source: 0x8014b900 FUN_8014b900
 */
void func_8014b900(s32 slot_index) {
  s32 slot_offset;

  slot_offset = slot_index << 7;
  *(u16*)((u8*)&DAT_80143b40 + slot_offset) = 0;
  func_8017ee0c();
  func_8017edac(*(s32*)((u8*)&DAT_80143b48 + slot_offset));
  func_8017ee1c();
}
