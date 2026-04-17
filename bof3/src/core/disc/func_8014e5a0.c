#include "internal.h"

/* clang-format off */
#include <libgte.h>
#include <libgpu.h>
/* clang-format on */

extern u8    DAT_80143d44;
extern u8*   DAT_8014598c;
extern void* DAT_801459d0[];

/* does: appends the current primitive to one OT head and advances the shared
 * primitive cursor when the requested byte count still fits in the active
 * buffer.
 * @source: 0x8014e5a0 FUN_8014e5a0
 */
void func_8014e5a0(u32 ot_index, u32 primitive_size) {
  u8* primitive;
  u8  size;

  primitive = DAT_8014598c;
  size = (u8)primitive_size;
  if ((u8*)(DAT_80143d44 * 0x9000 + 0x80028fcc) > (primitive + size)) {
    ot_index &= 0xff;
    CatPrim(DAT_801459d0[ot_index], primitive);
    DAT_801459d0[ot_index] = DAT_8014598c;
    DAT_8014598c += size;
  }
}
