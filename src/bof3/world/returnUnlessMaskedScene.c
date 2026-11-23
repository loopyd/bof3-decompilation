#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. The original materialises the cell
 * address with `lui` plus an offset load once and reads both the pointer word
 * at 0x0C and the step byte at 0x02 through that one loaded pointer, so the
 * source keeps a single pointer local.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Quad drawer of this target at 0x801F3F5C, which its own claimed source
 * defines. Only its prototype is declared here. Its head adds the third
 * argument to the work record byte at 0x0A and wraps the result modulo 0x0C,
 * and its fourth argument is subtracted from the horizontal and vertical
 * extents it derives from the work halfwords 0x36 and 0x3A. */
void renderPassTexturedSpread(
    u32 arg0, s32 arg1, u32 arg2, s32 arg3);

/* @source 0x801F3D94
 * @behavior Step 4 of the work-byte-2 handler run this overlay dispatches
 *           through the table at 0x801F43E8, whose fifth pointer word names
 *           this body. It follows the work record published through the
 *           scratchpad pointer cell 0x1F800044 to the record pointer held at
 *           work offset 0x0C, masks the 32-bit word that pointer addresses to
 *           0x00FFFF00 and compares the masked word against the scene state
 *           0x20800. When the masked word reads anything else it writes 2 into
 *           work record byte 0x02, the record byte the work-byte-2 dispatcher
 *           table indexes, which sends the next frame to step 2, the handler at
 *           0x801F3B84. It then calls the quad drawer at 0x801F3F5C with the
 *           fixed arguments (0xE0, 0x70, 2, 0). Takes no arguments, returns
 *           nothing, and its 0x18-byte frame holds only the saved return
 *           address at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void returnUnlessMaskedScene(
    void) {
  u8* work;

  work = D_1F800044;
  if ((*(u32*)*(u32*)(work + 0xC) & 0xFFFF00) != 0x20800) {
    work[2] = 2;
  }
  renderPassTexturedSpread(
      0xE0, 0x70, 2, 0);
}
