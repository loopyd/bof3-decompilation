#include "bof3/battle/battle03_internal.h"

#define BATTLE_SCRIPT_PTR                                                      \
  ((volatile u8*)FIELD_REF(volatile u8*, battleCurrentEnemyWorkState, 0xecu))

/* @source 0x801E2314
 * @behavior Reads mode byte +0x08 through the scratchpad pointer cell at
 * 0x1F800044. Modes 0 and 1 select script byte [arg0 & 0xFF]; modes 2 and 3
 * select the following byte. The script pointer comes from +0xEC of
 * battleCurrentEnemyWorkState. Calls func_8014D8D4 with the selected byte
 * masked to 0x7F, then rereads the script pointer and selected byte. Stores
 * bit 0x80 as 0 or 1 in work byte +0x2A through scratchpad +0x44 for modes 0
 * and 2, or its inverse for modes 1 and 3. Other modes return without calling
 * the helper or writing the flag.
 * @status exact
 * @match 100.00
 * @residual none
 */
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
