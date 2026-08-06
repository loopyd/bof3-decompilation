#include "internal.h"

/* @behavior scans one trigger table selected by `arg1` for the given byte id and,
 * on match, submits the associated effect id through `0x801636a0`.
 * @source 0x801DDF50
 */
u8 battle03_scan_trigger_table_submit(u16 arg0, u32 arg1) {
  u32* table;
  u32* entry;
  u16  id;
  u32  last;

  table = (u32*)D_800B6178[arg1 & 0xffu];
  if (table != (u32*)0) {
    entry = table;
    if (*entry != 0xffffffffu) {
      id = arg0 & 0xffu;
      last = 0xffffffffu;
    scan:
      if (*(u16*)((u8*)entry + 2) == id) {
        goto found;
      }
      entry += 1;
      if (*entry != last) {
        goto scan;
      }

    found:
      if (*entry != 0xffffffffu) {
        goto submit;
      }
    fail:
      return 1u;
    submit:
      func_801636A0((*entry & 0xffffu) + 0x1000u, 1u);
      return 0u;
    }
  }

  goto fail;
}
