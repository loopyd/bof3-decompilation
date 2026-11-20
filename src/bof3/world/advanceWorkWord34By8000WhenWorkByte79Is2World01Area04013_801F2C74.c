#include "bof3/bof3.h"

/* Body written from this span's own bytes: the 0x40-byte payload holds one
 * gate and one read-modify-write, and a masked scan of all 398 extracted
 * payloads under out/binaries found no other copy of the span, so no sibling
 * seeds these statements. The +0x79 byte of the work record published through
 * 0x80146250 is the mode selector this overlay family gates on (the exact
 * siblings advanceHalfword0aByCWhenWorkByte79Not8World00Area01313_801F2CAC and
 * advanceHalfword0aBy9WhenWorkByte79Not2World00Area01813_801F2C68 read the same
 * cell). The word at 0x34 of the record published through the scratchpad
 * pointer cell 0x1F800044 is that record's 16.16 x position: the exact sibling
 * seedWorkPositionSelectingYByWorkByte79ThenSetFacingByte8World03Area12813_801F3030
 * seeds 0x4E8000 and 0x80000 into the 0x34/0x38 pair, which are 78.5 and 8.0 in
 * that scale. */

/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x1F800044 @kind data */
extern u8* D_1F800044;

/* @source 0x801F2C74
 * @behavior Overlay per-frame handler: reads mode byte 0x79 of the work record
 *           published through the global pointer cell 0x80146250 and, only when
 *           that byte is 2, adds 0x8000 to the word at 0x34 of the record
 *           published through the scratchpad pointer cell 0x1F800044, storing
 *           the sum back through the one register that holds that record. The
 *           0x34 word is the record's 16.16 x position, so the step moves it by
 *           half a unit; every other mode returns before the cell is read. The
 *           0x8000 is materialised once into a register and added, the constant
 *           filling the branch delay slot. Takes no arguments, returns nothing,
 *           calls nothing and reserves no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWord34By8000WhenWorkByte79Is2World01Area04013_801F2C74(void) {
  u8* work;

  if (D_80146250[0x79] == 2) {
    work = D_1F800044;
    *(u32*)(work + 0x34) += 0x8000;
  }
}
