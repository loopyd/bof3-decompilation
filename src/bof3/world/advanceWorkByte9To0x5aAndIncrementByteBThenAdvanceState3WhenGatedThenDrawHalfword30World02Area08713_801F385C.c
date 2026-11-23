#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* scratchpadWorkRecord;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* The local emitter
 * drawSpriteFrameAndMainRamTexture0World02Area08713_801F40F8 narrows both
 * arguments with `sll`/`sra 16` in its own prologue and this call site consumes
 * no result. */
void drawSpriteFrameAndMainRamTexture0World02Area08713_801F40F8(
    s16 arg0, s16 arg1);

/* @source 0x801F385C
 * @behavior Entry 2 of the four-entry work-byte-3 handler run based at
 *           0x801F5760, whose third pointer word 0x801F5768 names this address
 *           and which the byte-3 dispatcher 0x801F36C0 indexes with record byte
 *           0x03, over the work record published at the scratchpad pointer cell
 *           0x1F800044. While record byte 0x0B is still zero it increments
 *           record byte 0x09 and, once that byte re-read through the pointer
 *           cell reaches 0x5A, increments record byte 0x0B as well. It then
 *           increments state byte 0x03 through another read of the cell when the
 *           shared mode byte 0x801454F2 is nonzero and record byte 0x0B is
 *           nonzero, or the phase byte 0x80143BB0 is 2, or bit 0x100 of
 *           halfword 0x80146258 is set. Finally it reads the cell once more and
 *           passes 0x5C and the signed halfword at record offset 0x30 to the
 *           local emitter 0x801F40F8. It takes no arguments, uses no result and
 *           saves only the return address in a 0x18-byte frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 0x100 bytes were compared against four sibling spans of this body under
 * out/binaries, at emi/world03/area115/13:0x801F382C,
 * emi/world01/area065/13:0x801F3860, emi/world03/area121/14:0x801F3840 and
 * emi/world00/area033/13:0x801F375C. Each differed in one word alone, span
 * offset 0xE8, the internal `jal` whose target is that overlay's own emitter
 * (0x801F40C8, 0x801F40FC, 0x801F40DC and 0x801F3FF8 against this overlay's
 * 0x801F40F8), so no span was byte-identical and the clone lever did not apply.
 * The reviewed advanceWorkByte9To0x5aAndIncrementByteBThenAdvanceState3WhenGatedThenDrawHalfword30World03Area11513_801F382C
 * supplied the statement order, operand forms and offset spelling of this body;
 * its literals, its record cells and its emitter address were written from this
 * span's own bytes, and the store of record byte 0x0B here is the read,
 * increment and store triple the bytes show.
 */
void advanceWorkByte9To0x5aAndIncrementByteBThenAdvanceState3WhenGatedThenDrawHalfword30World02Area08713_801F385C(
    void) {
  u8* record;
  u8* emitter;

  record = scratchpadWorkRecord;
  if (record[0x0B] == 0) {
    record[9]++;
    record = scratchpadWorkRecord;
    if (record[9] >= 0x5A) {
      record[0x0B]++;
    }
  }
  if ((D_801454F2 != 0 && scratchpadWorkRecord[0x0B] != 0) ||
      D_80143BB0 == 2 || (D_80146258 & 0x100u) != 0) {
    scratchpadWorkRecord[3]++;
  }
  emitter = scratchpadWorkRecord;
  drawSpriteFrameAndMainRamTexture0World02Area08713_801F40F8(
      0x5C, FIELD_REF(s16, emitter, 0x30));
}
