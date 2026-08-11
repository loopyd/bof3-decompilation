#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801ACE10
 * @behavior Dispatches a work callback selected by the active record or argument.
 */
void func_801ACE10(s32 arg0) {
  s32 index;
  void (**callbacks)(s32, s32);

  index = D_80146884[0x94] & 0x7F;
  if ((s8)index != 0x7F) {
    callbacks = *(void (***)(s32, s32))((u8 *)D_8017F974[D_80143F00] + 0x3C);
    callbacks[index](arg0, index);
  } else {
    (*(void (**)(s32, s32))((u8 *)D_801C890C + (s32)(s16)arg0 * 4))(arg0, index);
  }
}
