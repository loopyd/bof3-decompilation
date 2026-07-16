#include "internal.h"

extern u8 D_80140000[];

#define EMI_LOADER_STEP (*(volatile u32*)(D_80140000 + 0x646c))

extern vu8            D_80146483;
extern vu32           D_80146458;
extern EmiLoaderEntry D_8014677C[];
extern u32            D_80146788[];
extern u16            D_8014678E[];

/* @behavior selects the current EMI entry's alternate destination, marks the
 * entry with flag 2, advances the loader state machine, and advances the step.
 * @source 0x80162A6C
 */
void func_80162A6C(void) {
  u32 entry_offset;

  if (EMI_LOADER_STEP == 0) {
    entry_offset = D_80146483 * sizeof(EmiLoaderEntry);
    D_80146458 = *(u32*)((u8*)D_80146788 + entry_offset);
    *(u16*)((u8*)D_8014678E + entry_offset) |= 2;
  }

  func_80162C14();
  EMI_LOADER_STEP += 1;
}
