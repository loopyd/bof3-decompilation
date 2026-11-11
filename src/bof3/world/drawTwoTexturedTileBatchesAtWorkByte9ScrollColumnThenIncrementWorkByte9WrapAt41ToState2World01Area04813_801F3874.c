#include "bof3/bof3.h"

/* Unlifted eight-quad tile emitter in this overlay: its own bytes take a
 * halfword position, a texture-page value and an index byte, project eight
 * textured quads and submit them through the shared append helper. It stores
 * the incoming position straight into a halfword slot, so callers pass it
 * without truncation.
 */
void func_801F3AE4(s32 position, u32 texturePage, s16 index);

/* @source 0x801F3874
 * @behavior Overlay step handler, entry 1 of the five-handler work-byte-+0x02
 * run based at 0x801F4D80 that the byte-+0x02 dispatcher
 * dispatchStateByte2Table4d80World01Area04813_801F37B0 indexes. It reads the
 * scratch work record byte +0x09 as a scroll column and emits two batches of
 * eight textured tiles through the tile emitter at 0x801F3AE4: position
 * (column << 4) with texture page 0x858000 and index 0, or 0x10 once the column
 * has reached 0x31, then position ((column + 0x80) << 4) with texture page
 * 0x878000 and index 1, or 0x11 on that same threshold. It then increments the
 * work byte +0x09 and, once that byte reaches 0x41, increments the record
 * dispatch state byte +0x02 and clears work byte +0x09. Every access re-reads
 * the scratchpad pointer cell 0x1F800044. The function has a 0x18-byte frame
 * keeping the return address at +0x10, takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTwoTexturedTileBatchesAtWorkByte9ScrollColumnThenIncrementWorkByte9WrapAt41ToState2World01Area04813_801F3874(
    void) {
  u8** slots;
  u8 column;

  slots = SPAD_PTR_TABLE(u8);
  column = slots[0x11][0x09];
  func_801F3AE4(column << 4, 0x858000, column < 0x31 ? 0 : 0x10);
  slots = SPAD_PTR_TABLE(u8);
  column = slots[0x11][0x09];
  func_801F3AE4((column + 0x80) << 4, 0x878000, column >= 0x31 ? 0x11 : 1);
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11][0x09]++;
  slots = SPAD_PTR_TABLE(u8);
  if (slots[0x11][0x09] >= 0x41) {
    slots[0x11][0x02]++;
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x09] = 0;
  }
}
