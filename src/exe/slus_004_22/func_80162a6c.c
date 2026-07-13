#include "internal.h"

extern u8 DAT_80140000[];

#define EMI_LOADER_STEP (*(volatile u32*)(DAT_80140000 + 0x646c))

extern vu8            DAT_80146483;
extern vu32           DAT_80146458;
extern EmiLoaderEntry DAT_8014677c[];
extern u32            DAT_80146788[];
extern u16            DAT_8014678e[];

/* @behavior selects the current EMI entry's alternate destination, marks the
 * entry with flag 2, advances the loader state machine, and advances the step.
 * @source 0x80162a6c FUN_80162a6c
 */
void func_80162a6c(void) {
  u32 entry_offset;

  if (EMI_LOADER_STEP == 0) {
    entry_offset = DAT_80146483 * sizeof(EmiLoaderEntry);
    DAT_80146458 = *(u32*)((u8*)DAT_80146788 + entry_offset);
    *(u16*)((u8*)DAT_8014678e + entry_offset) |= 2;
  }

  func_80162c14();
  EMI_LOADER_STEP += 1;
}
