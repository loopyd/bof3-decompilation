#include "internal.h"

/* does: scans one trigger table selected by `arg1` for the given byte id and,
 * on match, submits the associated effect id through `0x801636a0`.
 * @source: 0x801ddf50 FUN_801ddf50
 */
u8 func_801ddf50(u16 arg0, u32 arg1) {
  u32* table;
  u32* entry;
  u16  id;

  table = (u32*)BOF3_BATTLE_TRIGGER_TABLE_6178[arg1 & 0xffu];
  if (table != (u32*)0) {
    entry = table;
    if (*entry != 0xffffffffu) {
      id = arg0 & 0xffu;

      while (*(u16*)((u8*)entry + 2) != id) {
        entry += 1;
        if (*entry == 0xffffffffu) {
          break;
        }
      }

      if (*entry != 0xffffffffu) {
        func_801636a0((*entry & 0xffffu) + 0x1000u, 1u);
        return 0u;
      }
    }
  }

  return 1u;
}
