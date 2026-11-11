#include "bof3/bof3.h"

/* Live work record published by the scratchpad pointer cell 0x1F800044. Only
 * the bytes this step touches are named; the cell is addressed as a symbol, not
 * as a literal constant, because the original forms the cell address again for
 * every access instead of keeping it in a saved register.
 * @source 0x1F800044 @kind unknown
 */
typedef struct World02Area08813Scratch {
  u8  unk_00;
  u8  state_01;
  u8  unk_02[0x09];
  u8  unk_0b;
  u32 unk_0c;
  u32 unk_10;
} World02Area08813Scratch;

extern volatile World02Area08813Scratch* D_1F800044;

/* Shared scroll fraction: the 16.16 scale both scroll words below are
 * multiplied by before the product is rounded to a halfword.
 * @source 0x80145E99 @kind unknown
 */
extern u8 D_80145E99;

/* First scroll word, scaled by the shared fraction above.
 * @source 0x80145E9C @kind unknown
 */
extern s32 D_80145E9C;

/* Second scroll word, scaled by the same fraction.
 * @source 0x80145EA0 @kind unknown
 */
extern s32 D_80145EA0;

/* Scroll base the first scaled word is added to.
 * @source 0x80145EC4 @kind unknown
 */
extern s32 D_80145EC4;

/* Scroll base the second scaled word is added to.
 * @source 0x80145EC8 @kind unknown
 */
extern s32 D_80145EC8;

/* Shared flag halfword whose bit 0x1000 selects the fourth state value.
 * @source 0x8014625A @kind unknown
 */
extern u16 D_8014625A;

/* Local handler table indexed by the work-record byte at offset 0x01.
 * @source 0x801F61A0 @kind unknown
 */
extern void (*D_801F61A0[])(void);

/* Resident map probe: takes the projected scroll position and returns the map
 * byte under it, which the original always masks to eight bits.
 * @source 0x80166CB0
 */
extern u8 func_80166CB0(s16 arg0, s16 arg1);

/* @source 0x801F3A50
 * @behavior Projects the shared scroll offsets at 0x80145E9C and 0x80145EA0,
 *           scaled by the shared byte at 0x80145E99, onto the scratch words at
 *           offsets 0x0C and 0x10 of the work record published at the
 *           scratchpad pointer cell 0x1F800044, then selects the record byte at
 *           offset 0x0B from three successive probe readings of that projected
 *           position (0xA1, 0xA0 and 0xAE), otherwise 4 when bit 0x1000 of the
 *           shared flag halfword 0x8014625A is set, otherwise 0, and finally
 *           dispatches the record byte at offset 0x01 through the local handler
 *           table based at 0x801F61A0. Every record access follows a fresh read
 *           of the pointer cell. Takes no arguments and returns nothing. The
 *           0x174-byte span equals the exact lifts at 0x801F2EF0 of the
 *           world03/area115 slot 13 overlay and at 0x801F2EEC of the
 *           world00/area016 slot 13 overlay word for word apart from the four
 *           jump-target words, which shift by exactly the span delta, and the
 *           handler-table offset word of the tail.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectScrollIntoScratchSelectByte0BThenDispatchModeWorld02Area08813_801F3A50(
    void) {
  World02Area08813Scratch* scratch;
  World02Area08813Scratch* state;
  s32 scale;
  s16 x;
  s16 y;
  u8 code;

  scale = D_80145E99;
  x = (D_80145EC4 + D_80145E9C * scale) >> 16;
  y = (D_80145EC8 + D_80145EA0 * scale) >> 16;

  scratch = D_1F800044;
  scratch->unk_0c = x;
  scratch->unk_10 = y;

  code = (u8)func_80166CB0(x, y);
  if (code == 0xA1) {
    state = D_1F800044;
    state->unk_0b = 1;
  } else {
    code = (u8)func_80166CB0(x, y);
    if (code == 0xA0) {
      state = D_1F800044;
      state->unk_0b = 2;
    } else {
      code = (u8)func_80166CB0(x, y);
      if (code == 0xAE) {
        state = D_1F800044;
        state->unk_0b = 3;
      } else if (D_8014625A & 0x1000) {
        state = D_1F800044;
        state->unk_0b = 4;
      } else {
        D_1F800044->unk_0b = 0;
      }
    }
  }

  D_801F61A0[D_1F800044->state_01]();
}
