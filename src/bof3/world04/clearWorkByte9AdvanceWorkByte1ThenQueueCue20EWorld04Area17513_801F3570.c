#include "bof3/bof3.h"

void func_8015DF18(u16 arg0);

/* @source 0x801F3570
 * @behavior Clears the byte at offset 0x09 of the scratchpad work object
 * published through the pointer cell at 0x1F800044, advances that object's
 * dispatch byte at offset 0x01 by one and then queues the front-end cue 0x20E
 * through func_8015DF18; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9AdvanceWorkByte1ThenQueueCue20EWorld04Area17513_801F3570(void) {
  u8 **slots;
  u8 *work;

  slots = SPAD_PTR_TABLE(u8);
  slots[0x11][9] = 0;
  work = slots[0x11];
  work[1]++;
  func_8015DF18(0x20E);
}
