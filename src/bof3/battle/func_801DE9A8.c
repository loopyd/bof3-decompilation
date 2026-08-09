#include "bof3/battle/battle03_internal.h"

/* @behavior submits one local battler template selected by byte `0x79` through the
 * common script/event helper rooted at `0x801490d8`.
 * @source 0x801DE9A8
 * @status partial
 * @match 58.06
 * @residual non-exact live audit: 18/28 instructions; 112 original bytes versus 124 current.
 */
void func_801DE9A8(u32 arg0) {
  func_801501E4(BATTLE_SCRIPT_TABLE_490D8,
                (void*)BATTLE_TEMPLATE_ABS_WORD_4968(
                    BATTLE_TABLE_81B10[BATTLE_LOCAL_BYTE_79(
                        &BATTLE_LOCAL_WORK_ARRAY[arg0 & 0xffu])]),
                5u);
}
