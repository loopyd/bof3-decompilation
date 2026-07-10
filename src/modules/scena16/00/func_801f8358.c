#include "internal.h"

/* does: dispatches one record callback selected by byte 0x7a.
 * @source: 0x801f8358 FUN_801f8358
 */
void func_801f8358(void* record) {
  Scena16RecordCallback* table;
  u8                     callback_index;
  u32                    arg1;

  table = (Scena16RecordCallback*)0x801f856cu;
  callback_index = ((const u8*)record)[0x7a];
  arg1 = ((vu32*)0x80140000u)[0x686c / 4];
  table[callback_index](record, arg1);
  __asm__ volatile("" ::: "memory");
}
