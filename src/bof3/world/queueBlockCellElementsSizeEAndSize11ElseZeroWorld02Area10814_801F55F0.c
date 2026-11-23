#include "bof3/bof3.h"

/* One cell coordinate pair of the four-cell block the payload keeps at
 * 0x801F63DC: two halfwords in a four-byte record, read by the original with
 * `lh` at the record base and `lh` at base + 2. */
typedef struct World02Area10814Cell63dc {
  s16 x;
  s16 y;
} World02Area10814Cell63dc;

/* The block's four cells, in the order the walk visits them: (0x33,0x3B),
 * (0x34,0x3B), (0x33,0x3C), (0x34,0x3C) - a two-by-four-byte block whose pairs
 * fill 0x801F63DC-0x801F63EB, up to the word the payload names 0x801F63EC. */
/* @source 0x801F63DC @kind table */
extern World02Area10814Cell63dc blockCellTable63World02[];

/* Shared three-argument helper of the main executable. */
/* @source 0x80155038 */
extern void func_80155038(s16 x, s16 y, u32 size);

/* Shared cue queue of the main executable. */
/* @source 0x801A4BC0 */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F55F0
 * @behavior Walks the four cells of the payload's block table at 0x801F63DC
 *           and hands each cell's coordinate pair to two shared helpers of the
 *           main executable: func_80155038 with the size 0xE and then
 *           func_801A4BC0 with the size 0x11. A zero byte argument runs the
 *           same walk with 0 as the size of both calls. Each pass loads the
 *           cell's two halfwords once (`lh` at the record base and at base + 2,
 *           offset by the index scaled by four) into $s0 and $s1 and reuses
 *           them for both calls, so no cell is re-read after a call. The index
 *           is masked to a byte before it is scaled and the walk ends when that
 *           byte reaches 4; the two size constants are loop invariant, so the
 *           size of the first helper is materialised once before the walk and
 *           rematerialised in the loop-back branch delay slot. Takes one byte
 *           argument, returns nothing and uses a 0x20-byte frame saving $ra,
 *           $s2 (the index), $s1 and $s0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueBlockCellElementsSizeEAndSize11ElseZeroWorld02Area10814_801F55F0(
    u8 arg0) {
  u8 i;
  s16 x;
  s16 y;

  if (arg0 != 0) {
    for (i = 0; i < 4; i++) {
      x = blockCellTable63World02[i].x;
      y = blockCellTable63World02[i].y;
      func_80155038(x, y, 0xE);
      func_801A4BC0(x, y, 0x11);
    }
  } else {
    for (i = 0; i < 4; i++) {
      x = blockCellTable63World02[i].x;
      y = blockCellTable63World02[i].y;
      func_80155038(x, y, 0);
      func_801A4BC0(x, y, 0);
    }
  }
}
