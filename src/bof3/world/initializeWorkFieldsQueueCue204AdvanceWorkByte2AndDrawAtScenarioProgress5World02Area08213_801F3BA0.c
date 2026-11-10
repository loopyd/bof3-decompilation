#include "bof3/bof3.h"

extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind data */
extern u8 D_80144ED8; /* @source 0x80144ED8 @kind data */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */
extern void func_8015B580(void* block, u8 index); /* @source 0x8015B580 */
extern void func_8015DF18(u32 cue); /* @source 0x8015DF18 */
extern void func_801F4430(void); /* @source 0x801F4430 */

/* @source 0x801F3BA0
 * @behavior At scenario progress 5, calls 0x8015B580 with the shared block
 *           at 0x80144ED8 and index 4, then initializes the scratch work
 *           record from pointer cell 0x1F800044. Sets word +0x3C to 0x3A0,
 *           word +0x14 to 0x53020, halfwords +0x30/+0x32 to 0x80/0,
 *           words +0x10/+0x0C to zero and byte +0x0B to 2. Saves the old
 *           +0x0C/+0x10 words, adds 0x4B8000/0x568000 with 32-bit wrapping,
 *           arithmetically shifts each by nine, subtracts 0x4000 and stores
 *           them at +0x34/+0x38. Re-reads the work cell to clear byte +0x09,
 *           queues cue 0x204, re-reads the cell to advance byte +0x02, and
 *           calls the work-record draw helper at 0x801F4430. Does nothing
 *           at other progress values. Takes no arguments and returns nothing.
 * @status partial
 * @match 60.00
 * @residual Register allocation and scheduling differ from offset 0x28; 33/55 instructions match at the original 220-byte size.
 */
void initializeWorkFieldsQueueCue204AdvanceWorkByte2AndDrawAtScenarioProgress5World02Area08213_801F3BA0(void) {
  u8* work;
  u8* tick;
  u8* step;
  s32 x;
  s32 y;

  if (g_ScenarioProgress == 5) {
    func_8015B580(&D_80144ED8, 4);
    work = SPAD_PTR_SLOT(u8, 0x44);
    *(s32*)(work + 0x3C) = 0x3A0;
    x = (s32)((u32)*(s32*)(work + 0x0C) + 0x4B8000u);
    y = (s32)((u32)*(s32*)(work + 0x10) + 0x568000u);
    *(s32*)(work + 0x14) = 0x53020;
    *(s16*)(work + 0x30) = 0x80;
    *(s16*)(work + 0x32) = 0;
    *(s32*)(work + 0x10) = 0;
    *(s32*)(work + 0x0C) = 0;
    work[0x0B] = 2;
    *(s32*)(work + 0x34) = (x >> 9) - 0x4000;
    *(s32*)(work + 0x38) = (y >> 9) - 0x4000;
    tick = workRecordCursor1F800044;
    tick[9] = 0;
    func_8015DF18(0x204);
    step = workRecordCursor1F800044;
    step[2]++;
    func_801F4430();
  }
}
