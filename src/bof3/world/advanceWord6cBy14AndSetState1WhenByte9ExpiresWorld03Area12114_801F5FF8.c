#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;

void func_801F5C7C(void);
void func_801F5E2C(void);

/* @source 0x801F5FF8
 * @behavior Turn-step update of the work record published at the scratchpad
 * pointer slot 0x1F800044. Decrements the countdown byte at 0x09 and advances
 * the word at 0x6C by the signed step word at 0x14, keeping the sum's low
 * twelve bits. It then reloads the cell and, when the reloaded countdown byte
 * has reached zero, writes 1 into the state byte at 0x01 and runs the local
 * settle helper at 0x801F5E2C, whose own path re-runs the aim test and ends in
 * the position update; otherwise it runs the local per-frame position update at
 * 0x801F5C7C directly. Takes no arguments and returns nothing. The word at 0x6C
 * is a twelve-bit accumulator stepped by 0x14 (the helper at 0x801F5ECC arms
 * that step to -0x40 or 0x40 and rearms byte 0x09 with 8); roles beyond those
 * offsets stay anchored by offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWord6cBy14AndSetState1WhenByte9ExpiresWorld03Area12114_801F5FF8(void) {
  u8* work;
  u8* reload;
  u8 countdown;
  u32 word;

  work = area12114WorkCursor_1F800044;
  word = FIELD_REF(u32, work, 0x6C);
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  reload = area12114WorkCursor_1F800044;
  FIELD_REF(u32, work, 0x6C) = (word + FIELD_REF(u32, work, 0x14)) & 0xFFFu;
  if (reload[9] == 0) {
    reload[1] = 1;
    func_801F5E2C();
  } else {
    func_801F5C7C();
  }
}
