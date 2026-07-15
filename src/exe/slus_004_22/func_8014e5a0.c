#include "internal.h"

extern u8    D_80143D44;
extern u8*   D_8014598C;
extern void* D_801459D0[];

/* @behavior appends the current primitive to one OT head and advances the shared
 * primitive cursor when the requested byte count still fits in the active
 * buffer.
 * @source 0x8014e5a0 FUN_8014e5a0
 */
void func_8014e5a0(u32 ot_index, u32 primitive_size) {
  u8* primitive;
  u8  size;

  primitive = D_8014598C;
  size = (u8)primitive_size;
  if ((u8*)(D_80143D44 * 0x9000 + 0x80028fcc) > (primitive + size)) {
    ot_index &= 0xff;
    CatPrim(D_801459D0[ot_index], primitive);
    D_801459D0[ot_index] = D_8014598C;
    D_8014598C += size;
  }
}
