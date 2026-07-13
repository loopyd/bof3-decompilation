#include "internal.h"

extern s32 DAT_8014646c;

/* @behavior copies the current type-0 EMI payload to RAM, then advances the
 * loader's completed-entry count.
 * @source 0x801625e4 func_801625e4
 */
void func_801625e4(void) {
  s32* completed_entries;

  func_80162c14();
  completed_entries = &DAT_8014646c;
  *completed_entries = *completed_entries + 1;
}
