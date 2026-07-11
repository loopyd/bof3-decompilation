#include "internal.h"

/* @behavior scans the 32-entry AREA028 work table and returns the first slot whose
 * active byte at offset `0` is clear, or `NULL` if none are free.
 * @source 0x801f3004 FUN_801f3004
 */
void* func_801f3004(void) {
  u8 i;

  WORLD00_AREA028_WORK_PTR = (World00Area028Work*)WORLD00_AREA028_WORK_BASE;
  i = 0u;
  do {
    if (WORLD00_AREA028_WORK_PTR->unk_00[0] != 0u) {
      (*(volatile World00Area028Work**)0x801f3e00u) =
          (World00Area028Work*)((u8*)WORLD00_AREA028_WORK_PTR + 0x10u);
      i += 1u;
    } else {
      return (void*)WORLD00_AREA028_WORK_PTR;
    }
  } while (i < 0x20u);

  return 0;
}
