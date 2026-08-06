#include "internal.h"

/* @source 0x8009EFB8
 * @behavior Selects the indexed battle-kind mask, invokes the selection helper,
 * and stores its signed result in the current selection record.
 */
void applyKindMaskSelection(void) {
  u16 index;
  u8 arg0;
  u16 mask;
  u8 arg1;
  s16 result;

  index = D_801463C0;
  arg0 = D_80146374;
  mask = D_801CA71C[index].mask & 0x1FFu;
  arg1 = D_80146394;
  result = func_801DC044(arg0, arg1, mask);
  D_801463A0[2] = result;
}
