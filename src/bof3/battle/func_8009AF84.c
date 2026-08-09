#include "bof3/battle/battle15_internal.h"

/* @behavior initializes the secondary battle selection-grid scratch band from the
 * saved group/page/cursor bytes.
 * @source 0x8009AF84
 * @status partial
 * @match 50.00
 * @residual non-exact live audit: 25/46 instructions; 184 original bytes versus 200 current.
 */
void func_8009AF84(void) {
  volatile u8* panel;
  u8           group;
  u8           page_base;
  u8           cursor;
  u8           rows;

  panel = BATTLE_LOCK_RAM_BASE;
  panel -= 0x7a90u;
  panel[0] = 1u;
  panel[3] = 2u;
  panel[8] = 2u;
  group = BATTLE_SELECTION_SAVED_GROUP(3);
  page_base = BATTLE_SELECTION_SAVED_SCROLL(3);
  cursor = BATTLE_SELECTION_SAVED_CURSOR(3);
  rows = 8u;
  panel[2] = 1u;
  panel[0xd] = 0xffu;
  *(volatile s16*)(panel + 4) = -0xaau;
  panel[1] = rows;
  panel[9] = 0u;
  *(volatile u16*)(panel + 0x10) = 0u;
  *(volatile u16*)(panel + 6) = 0x3fu;
  panel[0xa] = group;
  panel[0xb] = page_base;
  panel[0xc] = cursor;
  panel[0x6c] = 0u;
  panel[0x6d] = rows;
  panel[0x6e] = 0u;
}
