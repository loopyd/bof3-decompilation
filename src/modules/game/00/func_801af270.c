#include "internal.h"

extern u8 D_801CCE84[];
extern u8 D_801CCF7C[];

u8* func_801af270(u8 sprite_id, u8 flags) {
  if (flags & 0xff) return D_801CCF7C + (sprite_id & 0xff) * 4;
  return D_801CCE84 + (sprite_id & 0xff) * 4;
}
