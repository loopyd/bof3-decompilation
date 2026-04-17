#include "internal.h"

/* does: initializes the secondary battle selection-grid scratch band from the
 * saved group/page/cursor bytes.
 * @source: 0x8009af84 FUN_8009af84
 */
void func_8009af84(void) {
  volatile u8* panel;
  u8           group;
  u8           page_base;
  u8           cursor;
  u8           rows;

  panel = (volatile u8*)0x80150000u;
  panel -= 0x7a90u;
  panel[0] = 1u;
  panel[3] = 2u;
  panel[8] = 2u;
  group = *(volatile u8*)0x801454fdu;
  page_base = *(volatile u8*)0x801454feu;
  cursor = *(volatile u8*)0x801454ffu;
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
