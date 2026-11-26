#include "bof3/bof3.h"

/* Work-record table of the shared main-executable data segment: 0x140-byte rows
 * based at 0x80145E90, whose leading byte the original tests before it reads any
 * of the row's own fields. The two words this body reads sit at row offsets 0x34
 * (0x80145EC4) and 0x38 (0x80145EC8). All three bases lie outside this overlay's
 * payload; Splat binds them through this target's generated undefined-syms map,
 * so no target-map row is needed. The original re-materialises the base of each
 * access and adds the scaled row offset, so each field base keeps its own
 * address-derived symbol and a byte view carries the scaled row offset.
 * @source 0x80145E90 @kind table
 */
extern u8 D_80145E90[];
/* @source 0x80145EC4 @kind data */
extern u8 D_80145EC4[];
/* @source 0x80145EC8 @kind data */
extern u8 D_80145EC8[];

/* Scratchpad work cursor of this overlay, holding the record the byte-2 state
 * handlers below read and advance. The cell itself is outside the payload and
 * this target's map owns it.
 * @source 0x1F800044 @kind unknown
 */
extern u8* workRecordCursor1F800044;

/* Pair helper of this payload, still an `asm` span at 0x801F4158: it takes a
 * slot index and a signed mode byte and answers nothing.
 * @source 0x801F4158 @kind function
 */
extern void func_801F4158(s32 index, s8 mode);

/* @source 0x801F3FBC
 * @behavior Fourth handler of the state-byte-2 run based at 0x801F4BA0, at word
 *           0x801F4BAC. That word is the only mention of this address in the
 *           payload - no jal names it - so it takes no arguments and returns no
 *           value. Walks the first three 0x140-byte work records based at
 *           0x80145E90 and skips any record whose leading byte reads zero; for
 *           the rest it takes the absolute distance between the record word at
 *           +0x38 and the work cursor halfword at +0x3A scaled by 0x10000 and
 *           skips the record once that distance passes 0x28000. The surviving
 *           record then compares the cursor halfword at +0x36 with the record
 *           word at +0x34 the same way: the record qualifies when the distance
 *           from the cell start stays within 0x20000 or, failing that, when the
 *           distance from the next cell start - the cursor halfword plus one,
 *           scaled by 0x10000 - stays within 0x20000. One qualifying record ends
 *           the walk with the found flag set. When no record qualified the body
 *           re-reads the scratchpad cursor cell and increments the record's own
 *           byte +2 rather than reusing the copy the walk already held, then
 *           calls the local helper 0x801F4158 with (0, 0) and (1, 3) on every
 *           path. Takes no arguments, answers nothing, and its 0x18-byte frame
 *           keeps $ra at 0x10. The roles of the two halfwords and of the two
 *           window widths stay address-anchored.
 *
 * Representation notes: the first seed spelled each distance as
 * `if (d < 0) { d = -d; }` and each guard as `if (d > 0x28000)`. That spelling
 * measured 34/71 instructions: it needed one register more than the original, so
 * every local sat one place higher (the found flag in $9 rather than $8) and the
 * distance branches carried a filled delay slot. Taking each distance through
 * the integer absolute-value intrinsic and assigning each comparison back into
 * that same distance measured 71/71 and matched the payload bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateNoInsideCamera(
    void) {
  u8* work;
  u8* cursor;
  s32 found;
  s32 i;
  s32 row;
  s32 d;
  s32 value;
  s32 base;

  found = 0;
  work = workRecordCursor1F800044;
  for (i = 0; i < 3; i++) {
    row = i * 0x140;
    if (D_80145E90[row] == 0) {
      continue;
    }
    value = FIELD_REF(s16, work, 0x3A);
    d = *(s32*)(D_80145EC8 + row) - (value << 16);
    d = __builtin_abs(d);
    d = d > 0x28000;
    if (d) {
      continue;
    }
    value = FIELD_REF(s16, work, 0x36);
    base = *(s32*)(D_80145EC4 + row);
    d = base - (value << 16);
    d = __builtin_abs(d);
    d = d > 0x20000;
    if (d) {
      d = base - ((value + 1) << 16);
      d = __builtin_abs(d);
      d = d > 0x20000;
      if (d) {
        continue;
      }
    }
    found = 1;
  }
  if (!found) {
    cursor = workRecordCursor1F800044;
    cursor[2]++;
  }
  func_801F4158(0, 0);
  func_801F4158(1, 3);
}
