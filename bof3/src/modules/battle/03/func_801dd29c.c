#include "internal.h"

/* does: advances through the ranked owner list, assigns the next valid owner to
 * `0x801462ee`, and marks that owner's local bytes `0x118/0x119` for followup
 * processing.
 * @source: 0x801dd29c FUN_801dd29c
 */
void func_801dd29c(void) {
  volatile s8* global_byte_6303;
  u8           owner;

  global_byte_6303 = (volatile s8*)0x80146303;
  while (*global_byte_6303 < 3) {
    owner = *(volatile u8*)(0x8014630cu + (s32)*global_byte_6303);
    if (owner == 0xffu) {
      break;
    }
    BOF3_BATTLE_GLOBAL_BYTE_62EE = owner;
    BOF3_BATTLE_LOCAL_BYTE_119(&BOF3_BATTLE_LOCAL_WORK_ARRAY[owner]) = 1u;
    BOF3_BATTLE_LOCAL_BYTE_118(
        &BOF3_BATTLE_LOCAL_WORK_ARRAY[BOF3_BATTLE_GLOBAL_BYTE_62EE]) = 3u;
    *global_byte_6303 += 1u;
  }
}
