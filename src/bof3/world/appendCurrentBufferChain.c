#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared main-RAM cells this overlay reaches: the frame-buffer index byte, the
 * per-buffer packet-chain slot run and the primitive cursor. */
extern u8 D_80143D44;
extern u8 D_80142F94[];

/* @source 0x801F3288
 * @behavior Claims the packet at the shared primitive cursor for the current
 * frame buffer: the byte argument is the packet size, and the body runs only
 * while cursor + size stays below the arena limit
 * D_80143D44 * 0x9000 + 0x80028FCC. The slot at
 * D_80142F94 + ((D_80143D44 + 4) << 3) holds the previously claimed packet; its
 * leading word keeps its top byte and takes the low 24 bits of the newly
 * claimed packet address, the cursor advances by the size byte, and the slot is
 * republished as the new packet. Nothing is written when the arena is full. No
 * call and no stack frame. Its only in-image references are the four jal sites
 * in the dispatcher at 0x801F2C90, which pass the 0x28, 0x24 and 0xC byte sizes
 * of the primitives they have just written at the cursor.
 * @status exact
 * @match 100.00
 * @residual none
 */
void appendCurrentBufferChain(
    u32 primitive_size) {
  u32 offset;
  u8 size;
  u8 *primitive;
  u32 *cell;

  primitive = g_PrimCursor;
  size = (u8)primitive_size;
  if ((u8 *)(D_80143D44 * 0x9000 + 0x80028FCC) > (primitive + size)) {
    offset = (D_80143D44 + 4) << 3;
    cell = *(u32 **)(D_80142F94 + offset);
    g_PrimCursor = primitive + size;
    *cell = (*cell & 0xFF000000) | ((u32)primitive & 0xFFFFFF);
    *(u8 **)(D_80142F94 + offset) = primitive;
  }
}
