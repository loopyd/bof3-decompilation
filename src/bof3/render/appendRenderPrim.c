#include "bof3/core/slus_internal.h"

extern u8    D_80143D44;
extern u8*   g_PrimCursor;
extern void* bootOrderingTableHeads[];

/* @behavior appends the current primitive to one OT head and advances the shared
 * primitive cursor when the requested byte count still fits in the active
 * buffer.
 * @source 0x8014E5A0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void appendRenderPrim(u32 ot_index, u32 primitive_size) {
  u8* primitive;
  u8  size;

  primitive = g_PrimCursor;
  size = (u8)primitive_size;
  if ((u8*)(D_80143D44 * 0x9000 + 0x80028fcc) > (primitive + size)) {
    ot_index &= 0xff;
    CatPrim(bootOrderingTableHeads[ot_index], primitive);
    bootOrderingTableHeads[ot_index] = g_PrimCursor;
    g_PrimCursor += size;
  }
}
