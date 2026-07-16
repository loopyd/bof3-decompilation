#include "internal.h"

/* @behavior dispatches through the primary SCENA16 state table.
 * @source 0x801F6C90
 */
void func_801F6C90(void) {
  s8*              state_base;
  Scena16Callback* table;

  state_base = (s8*)0x80140000u;
  table = (Scena16Callback*)0x801f854cu;
  table[state_base[0x6872]]();
}
