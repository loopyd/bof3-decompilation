#include "bof3/ui/game00_internal.h"

/* @behavior Finds the first eight-byte bounding-box record in the list reached
 * from the pointer table D_80181164[index], where index is the pending world id
 * D_80143F00, and returns a pointer to the record whose box contains the point
 * (x, y): record[0] <= x, record[1] <= y, record[2] >= x and record[3] >= y.
 * The scan advances both the record pointer and its box-end pointer eight bytes
 * at a time with no count bound, so the box list is expected to contain the
 * point.
 * @source 0x801B3C3C
 * @status partial
 * @match 77.78
 * @residual exit-path branch polarity and return-copy placement; all four box tests and the preamble match byte-for-byte
 * Live asm-diff reports 28/36 instructions, 136 versus 144 bytes, first
 * difference at +0x30: the compiler inverts the fourth (record[3] < py) test
 * into `beqz $v0, epilogue` and leaves the return-value copy inside the
 * epilogue, where the original keeps the taken-to-latch `bnez`, duplicates the
 * box-end increment into its delay slot and returns through a `j` epilogue.
 * Every tested spelling that preserves that branch shape (conjunction,
 * nested ifs, goto, while/break) instead makes loop.c materialize a second
 * derived `bounds - 1` register (24/38, 152 bytes). Next untried rung is a
 * per-object compiler-profile probe or the permuter; both are opt-in and are
 * not authorized for this mission.
 */
u8* func_801B3C3C(u8 x, u8 y) {
  u8* record;
  u8* bounds;
  s32 px;
  s32 py;

  record = D_80181164[D_80143F00];
  bounds = record + 3;
  px = x;
  py = y;
  for (;;) {
    if (px < record[0]) {
      bounds += 8;
      record += 8;
      continue;
    }
    if (py < bounds[-2]) {
      bounds += 8;
      record += 8;
      continue;
    }
    if (bounds[-1] < px) {
      bounds += 8;
      record += 8;
      continue;
    }
    if (bounds[0] < py) {
      bounds += 8;
      record += 8;
      continue;
    }
    return record;
  }
}
