#include "bof3/bof3.h"

/* Scratchpad pointer cell of the work record this step advances. The original
 * re-reads the cell through its own address load for each of the three step
 * groups, so every group re-reads the symbol instead of reusing one pointer.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Pointer cell of the shared record whose halfword at 0x0A this step consumes.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* @source 0x801F3320
 * @behavior Per-frame fall step of the scratch work record published through
 *           the pointer cell at 0x1F800044: adds the record step word at 0x0C
 *           to the accumulator word at 0x34, adds -0x2000 to each of the words
 *           at 0x40 and 0x44, and decrements the countdown byte at 0x0A. While
 *           the countdown is a multiple of four it stores 1 into work byte 4,
 *           the table index of the rise step at 0x801F3278 in the handler
 *           table at 0x801F41DC that
 *           dispatchWorkByte4Table41dcWorld04Area19113_801F31A0 indexes; that
 *           rise step repeats this body with +0x2000 and selects 2 again, so
 *           the two alternate every fourth tick and the words at 0x40 and 0x44
 *           fall and rise together. Once the countdown reaches zero it instead
 *           stores 0 into work byte 4, which returns the record to the arming
 *           handler at 0x801F31E4, and that arm leaves the halfword at 0x0A of
 *           the record published through the global pointer cell 0x80148210
 *           untouched; while any countdown remains the step consumes two units
 *           of that halfword. The second read of the 0x1F800044 cell sits
 *           before the 0x44 update because the original keeps that pointer in
 *           its own register ahead of the last store. Takes no arguments, calls
 *           nothing and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkStepAndLowerWordPair40And44By2000ThenHandOffEveryFourthTickWorld04Area19113_801F3320(
    void) {
  u8* work;
  u8* state;
  u8* next;
  u8 count;

  work = D_1F800044;
  count = work[0x0a] - 1;
  *(u32*)(work + 0x34) += *(u32*)(work + 0x0c);
  *(u32*)(work + 0x40) -= 0x2000;
  work[0x0a] = count;
  state = D_1F800044;
  *(u32*)(work + 0x44) -= 0x2000;

  if ((state[0x0a] & 3) == 0) {
    state[0x04] = 1;
  }

  next = D_1F800044;
  if (next[0x0a] == 0) {
    next[0x04] = 0;
  } else {
    D_80148210[5] -= 2;
  }
}
