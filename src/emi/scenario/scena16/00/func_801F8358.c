#include "internal.h"

/* @behavior dispatches one record callback selected by byte 0x7a.
 * @source 0x801F8358
 */
void func_801F8358(void* record) {
  Scena16RecordCallback* table;
  u8                     callback_index;
  u32                    arg1;

  table = SCENA16_PTR_801F856C;
  callback_index = ((const u8*)record)[0x7a];
  arg1 = PSX_PTR(volatile u32, 0x80140000u)[0x686c / 4];
  table[callback_index](record, arg1);
}
