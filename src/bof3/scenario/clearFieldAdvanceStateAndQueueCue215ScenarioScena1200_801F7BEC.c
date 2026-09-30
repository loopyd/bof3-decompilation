#include "bof3/bof3.h"

void func_8015DF18(u16 arg0);

/* @source 0x801F7BEC
 * @behavior Overlay per-frame work handler: it reads the scratchpad work
 * object published by the pointer cell at 0x1F800044, saves the object's state
 * byte at offset 0x01, clears the object's halfword at offset 0x2E, stores that
 * saved state byte + 1 back to offset 0x01 and queues the front-end cue 0x215
 * through func_8015DF18(u16). It takes no arguments, returns nothing and
 * touches no state other than those two object fields; the 0x18-byte frame only
 * keeps $ra across the call. The address is table entry 8 of the overlay's
 * per-frame handler table at 0x801FD178 (the word read at the table word
 * 0x801FD198), which the overlay dispatcher func_801F7174 selects with the
 * work-object state byte at offset 0x01 (it reads that byte, shifts it left by
 * two and calls `jalr` on the table word); the state byte this function
 * advances therefore moves the overlay from state 8 to state 9, the next table
 * word 0x801F7C24. The name records that proven behavior: clear the object's
 * field at 0x2E, advance its state byte, queue cue 0x215.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearFieldAdvanceStateAndQueueCue215ScenarioScena1200_801F7BEC(void) {
  u8 *work;
  u8 step;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  step = work[1];
  *(u16 *)(work + 0x2E) = 0;
  work[1] = step + 1;
  func_8015DF18(0x215);
}
