#include "internal.h"

/* does: dispatches the current preview-sequence state under one temporary
 * global word setting, then restores the fixed word value.
 * @source: 0x801e6c84 FUN_801e6c84
 */
void func_801e6c84(void) {
  struct PreviewSequenceTable {
    Battle03Handler entries[5];
  } table;
  struct PreviewSequenceTable const volatile* src;

  src = (struct PreviewSequenceTable const volatile*)
      BOF3_BATTLE_PREVIEW_SEQUENCE_TABLE;
  table = *src;

  if (BOF3_BATTLE_GLOBAL_BYTE_62E0 != 5u) {
    *(volatile u32*)0x801459f0u = 0x800f0800u;
    table.entries[BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_01]();
    *(volatile u32*)0x801459f0u = 0x800d3800u;
  }
}
