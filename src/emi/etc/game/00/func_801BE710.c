#include "internal.h"

/* @behavior clears the observed reset fields and sets the state byte to 3.
 * @source 0x801BE710
 */
void func_801BE710(GameResetRecord* record) {
  record->unk_0A = 0;
  record->unk_07 = 0;
  record->unk_02 = 0;
  record->unk_05 = 0;
  record->unk_00 = 0;
  record->unk_01 = 0;
  record->unk_03 = 0;
  record->unk_04 = 3;
}
