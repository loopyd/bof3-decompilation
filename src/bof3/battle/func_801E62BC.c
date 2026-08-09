#include "bof3/battle/battle03_internal.h"

/* @behavior rolls the current queued-slot digit counter and redraws each digit with
 * `func_801D9684`, using the queued-slot x/y/count fields.
 * @source 0x801E62BC
 * @status partial
 * @match 62.69
 * @residual non-exact live audit: 42/65 instructions; 260 original bytes versus 268 current.
 */
void func_801E62BC(u8 arg0) {
  s16 counter;
  s16 current_digit;
  s16 y;
  s8  digit_index;

  counter =
      (s16)(*(volatile u16*)((volatile s8*)BATTLE_CURRENT_QUEUED_SLOT_PTR +
                             0x32) +
            1);
  digit_index = ((volatile s8*)BATTLE_CURRENT_QUEUED_SLOT_PTR)[0x0a];
  *(volatile s16*)((volatile s8*)BATTLE_CURRENT_QUEUED_SLOT_PTR + 0x32) =
      (s16)(counter % 10);

  while (digit_index >= 0) {
    current_digit =
        *(volatile s16*)((volatile s8*)BATTLE_CURRENT_QUEUED_SLOT_PTR + 0x32);
    y = *(volatile s16*)((volatile s8*)BATTLE_CURRENT_QUEUED_SLOT_PTR + 0x3a);
    func_801D9684(
        (s16)(*(volatile u16*)((volatile s8*)BATTLE_CURRENT_QUEUED_SLOT_PTR +
                               0x36) +
              (digit_index * -8) - 0x0cu),
        y, arg0, (u16)((current_digit + digit_index) % 10));
    digit_index -= 1;
  }
}
