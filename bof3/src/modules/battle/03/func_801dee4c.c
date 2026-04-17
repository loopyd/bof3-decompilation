#include "internal.h"

/* does: returns ready immediately when the local work flags allow it; otherwise
 * delegates to the second EXE-side readiness helper.
 * @source: 0x801dee4c FUN_801dee4c
 */
u8 func_801dee4c(void) {
  volatile u8* battle_work = (volatile u8*)BOF3_BATTLE_LOCAL_WORK_PTR;
  volatile u8* global_base = (volatile u8*)0x80140000u;

  if ((*(volatile u16*)(battle_work + 0x80) & 4u) != 0u) {
    return 1u;
  }

  if ((*(global_base + 0x63ce) != 0u) &&
      ((*(volatile u32*)(battle_work + 0x128) & 0x10u) == 0u)) {
    return 1u;
  }

  return func_8014daec();
}
