#include "internal.h"

/* @behavior dispatches through the primary SCENA16 state table.
 * @source 0x801F6C90
 */
void func_801F6C90(void) {
  s8*              state_base;
  Scena16Callback* table;

  state_base = PSX_PTR(s8, 0x80140000u);
  table = SCENA16_PTR_801F854C;
  table[state_base[0x6872]]();
}
