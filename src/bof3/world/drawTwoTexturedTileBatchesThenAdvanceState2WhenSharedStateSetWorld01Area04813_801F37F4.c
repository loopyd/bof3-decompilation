#include "bof3/bof3.h"

/* Unlifted eight-quad tile emitter in this overlay: its own bytes take a
 * halfword position, a texture-page value and an index byte, project eight
 * textured quads and submit them through the shared append helper.
 */
void func_801F3AE4(s16 position, u32 texturePage, s16 index);

/* Shared overlay state byte at 0x8014933B. Other overlays raise it to 1 and
 * clear it back to 0, and this step advances the dispatch state byte only
 * while it is nonzero.
 * @source 0x8014933B @kind data
 */
extern u8 D_8014933B;

/* @source 0x801F37F4
 * @behavior Overlay step handler, entry 0 of the five-handler work-byte-+0x02
 * run based at 0x801F4D80 that the byte-+0x02 dispatcher
 * dispatchStateByte2Table4d80World01Area04813_801F37B0 indexes. It emits two
 * batches of eight textured tiles through the tile emitter at 0x801F3AE4:
 * position 0 with texture page 0x858000 and index 0, then position 0x800 with
 * texture page 0x878000 and index 1. While the shared overlay state byte at
 * 0x8014933B is nonzero it then advances the scratch work record's dispatch
 * state byte +0x02 by one and clears that record's byte +0x09, re-reading the
 * scratchpad pointer cell 0x1F800044 for the second store. The function has a
 * 0x18-byte frame keeping the return address at +0x10, takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTwoTexturedTileBatchesThenAdvanceState2WhenSharedStateSetWorld01Area04813_801F37F4(
    void) {
  u8** slots;

  func_801F3AE4(0, 0x858000, 0);
  func_801F3AE4(0x800, 0x878000, 1);
  if (D_8014933B != 0) {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][2]++;
    slots[0x11][9] = 0;
  }
}
