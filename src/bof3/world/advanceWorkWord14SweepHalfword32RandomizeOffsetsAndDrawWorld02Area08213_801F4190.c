#include "bof3/bof3.h"

extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */
extern u8 g_ScenarioProgress;
extern s32 rand(void);
extern void func_801F4430(void);

/* @source 0x801F4190
 * @behavior Work-byte-2 handler 6 in the table at 0x801F576C. Advances the
 *           signed work word +0x14 by q*(q-10), where q is its signed quotient
 *           by 10000, and subtracts the same quotient from halfword +0x32,
 *           masking with 0xFFD. A change in bit 0x800 against halfword +0x2E
 *           queues byte +0x0B as the signed new halfword shifted by eleven,
 *           plus one. Copies +0x32 to +0x2E. In the unsigned word window
 *           680000..1020000, randomizes words +0x0C/+0x10 with rand & 0x1000;
 *           above signed 1019999, uses rand & 0x2000 and advances scenario
 *           progress from 6 to 7. Above signed 1369999, advances work byte 2.
 *           Always calls the work-record draw helper at 0x801F4430. Takes no
 *           arguments and returns nothing; unknown record roles stay offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWord14SweepHalfword32RandomizeOffsetsAndDrawWorld02Area08213_801F4190(void) {
  u8* work;
  s32 value;
  s32 quotient;
  u8* band;
  u8* step;
  s32 roll;
  u8* progress;

  work = workRecordCursor1F800044;
  value = *(s32*)(work + 0x14);
  quotient = value / 10000;
  value += quotient * (quotient - 10);
  *(u16*)(work + 0x32) =
      (*(u16*)(work + 0x32) - *(s32*)(work + 0x14) / 10000) & 0xFFD;
  *(s32*)(work + 0x14) = value;
  if ((*(u16*)(work + 0x2E) & 0x800) !=
      (*(u16*)(work + 0x32) & 0x800)) {
    work[0xB] = (*(s16*)(work + 0x32) >> 11) + 1;
  }
  band = workRecordCursor1F800044;
  *(u16*)(band + 0x2E) = *(u16*)(band + 0x32);
  if ((u32)(*(s32*)(band + 0x14) - 680000) <= 340000u) {
    roll = rand();
    *(s32*)(workRecordCursor1F800044 + 0xC) = roll & 0x1000;
    roll = rand();
    *(s32*)(workRecordCursor1F800044 + 0x10) = roll & 0x1000;
  }
  if (*(s32*)(workRecordCursor1F800044 + 0x14) > 1019999) {
    roll = rand();
    *(s32*)(workRecordCursor1F800044 + 0xC) = roll & 0x2000;
    roll = rand();
    *(s32*)(workRecordCursor1F800044 + 0x10) = roll & 0x2000;
    progress = &g_ScenarioProgress;
    if (*progress == 6) {
      (*progress)++;
    }
  }
  step = workRecordCursor1F800044;
  if (*(s32*)(step + 0x14) > 1369999) {
    step[2]++;
  }
  func_801F4430();
}
