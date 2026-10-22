#include "bof3/battle/battle03_internal.h"

/* @source 0x801E2314
 * @behavior Consumes one enemy script byte selected by the scratch work mode
 * byte +0x08 and derives the flag byte +0x2A of that same scratch record from
 * it: modes 0 and 1 read script byte [index] and modes 2 and 3 read script byte
 * [index + 1]; the byte masked to 0x7F is handed to func_8014D8D4 and the flag
 * takes bit 0x80 of the byte for modes 0 and 2 and the inverse of that bit for
 * modes 1 and 3. Any other mode leaves the flag untouched.
 * @status exact
 * @match 100.00
 * @residual none
 */

#define BATTLE_SCRIPT_PTR                                                      \
  ((volatile u8*)FIELD_REF(volatile u8*, battleCurrentEnemyWorkState, 0xecu))

void func_801E2314(u32 arg0) {
  s32 mode;
  u8  value;

  mode = D_1F800044->unk_08;
  switch (mode) {
  case 0:
    func_8014D8D4(BATTLE_SCRIPT_PTR[arg0 & 0xff] & 0x7f);
    value = BATTLE_SCRIPT_PTR[arg0 & 0xff];
    if (value & 0x80) {
      D_1F800044->unk_2a = 1;
    } else {
      D_1F800044->unk_2a = 0;
    }
    break;
  case 1:
    func_8014D8D4(BATTLE_SCRIPT_PTR[arg0 & 0xff] & 0x7f);
    value = BATTLE_SCRIPT_PTR[arg0 & 0xff];
    if (value & 0x80) {
      D_1F800044->unk_2a = 0;
    } else {
      D_1F800044->unk_2a = 1;
    }
    break;
  case 2:
    func_8014D8D4((BATTLE_SCRIPT_PTR + (arg0 & 0xff))[1] & 0x7f);
    value = (BATTLE_SCRIPT_PTR + (arg0 & 0xff))[1];
    if (value & 0x80) {
      D_1F800044->unk_2a = 1;
    } else {
      D_1F800044->unk_2a = 0;
    }
    break;
  case 3:
    func_8014D8D4((BATTLE_SCRIPT_PTR + (arg0 & 0xff))[1] & 0x7f);
    value = (BATTLE_SCRIPT_PTR + (arg0 & 0xff))[1];
    if (value & 0x80) {
      D_1F800044->unk_2a = 0;
    } else {
      D_1F800044->unk_2a = 1;
    }
    break;
  }
}
