#include "bof3/bof3.h"

/* Cue dispatcher of the main executable: plays the cue its id names. */
extern void func_8015DF18(u16 cue); /* @source 0x8015DF18 */

/* Action starter of the main executable: runs the action the id names. */
extern void func_80150224(s32 action_id); /* @source 0x80150224 */

/* Frontend mode byte of the main executable; 2 marks the frontend busy. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Step group of this overlay: byte 0 selects the handler run the step
 * dispatcher indexes, byte 1 the handler of its byte-1 view. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */

/* @source 0x801D2E10
 * @behavior Transition body of this overlay's commu card screen. It queues
 *           cue 0x106 through the main-executable cue dispatcher
 *           func_8015DF18 and starts action 0x72 through the action starter
 *           func_80150224. It then latches the frontend mode byte at
 *           0x80143BB0 to 2 (busy), clears the byte-1 step index at 0x801F294D
 *           that setFrontLocalMode0AndAdvanceStepByte1 advances, and advances
 *           byte 0 of the step group at 0x801F294C by one so the byte-0
 *           dispatcher runs the next handler of the ten-entry run at
 *           0x801DAD68. The byte-0 cell is read once, before the two stores to
 *           the other cells, and written back with its own incremented value.
 *           Nothing is returned and no other cell is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCue106StartAction72LatchFrontendMode2ResetStepByte1AndAdvanceStepByte0(
    void) {
  u8 step;

  func_8015DF18(0x106);
  func_80150224(0x72);
  step = D_801F294C;
  D_80143BB0 = 2;
  D_801F294D = 0;
  D_801F294C = step + 1;
}
