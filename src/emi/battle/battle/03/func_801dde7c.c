#include "internal.h"

/* @behavior advances one byte counter in the selected `0x801c8950` table and stores
 * the caller's packed byte into the parallel table at `0x801c893c`.
 * @source 0x801DDE7C
 */
u8 func_801DDE7C(u32 arg0, u32 arg1) {
  u32   table_index;
  char* counter_table;
  char* value_table;
  char* ptr;
  char  counter;

  table_index = (arg1 >> 6) & 0x3fcu;
  counter_table = ((char**)0x801c8950)[table_index >> 2];
  ptr = counter_table + (arg0 & 0xffu);
  counter = *ptr;
  if (counter == 'c') {
    return 1u;
  }
  *ptr = counter + 1;
  value_table = ((char**)0x801c893c)[table_index >> 2];
  value_table[arg0 & 0xffu] = arg1;
  return 0u;
}
