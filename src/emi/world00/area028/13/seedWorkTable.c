#include "internal.h"

/* @behavior seeds the local 32-entry AREA028 work table and clears the active byte
 * at offset `0` for each `0x10`-byte slot.
 * @source 0x801F2F5C
 */
void seedWorkTable(void) {
  u8 i;

  WORLD00_AREA028_WORK_PTR = (World00Area028Work*)WORLD00_AREA028_WORK_BASE;
  i = 0u;
  do {
    WORLD00_AREA028_WORK_PTR->unk_00[0] = 0u;
    WORLD00_AREA028_WORK_PTR =
        (World00Area028Work*)((u8*)WORLD00_AREA028_WORK_PTR + 0x10u);
    i += 1u;
  } while (i < 0x20u);
}
