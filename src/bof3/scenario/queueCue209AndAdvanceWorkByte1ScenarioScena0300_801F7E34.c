#include "bof3/bof3.h"

void func_8015DF18(u16 arg0);

/* @source 0x801F7E34
 * @behavior Entry 0 of the overlay's work-object handler table at 0x801FCF88
 * (the three words 0x801F7E34 / 0x801F7E6C / 0x801F7F84), which the frame
 * dispatcher func_801F7DF0 selects with byte 0x1 of the work object published
 * at scratchpad pointer slot 0x1F800044 (it reads that byte, shifts it left by
 * two and calls `jalr` on the selected table word) and calls with no
 * arguments: it queues the front-end cue 0x209 through func_8015DF18 and then
 * advances that work byte by one, so the next frame dispatches entry 1
 * (0x801F7E6C, which claims five records from func_8019601C into the table at
 * 0x80143FC8 and resolves its own tail by advancing the same byte again). It
 * reads no state other than the pointer cell and writes only that work byte;
 * it takes no arguments, returns nothing of its own and the 0x18-byte frame
 * exists only to hold $ra across the cue call, which the pointer reload follows
 * because the cue dispatcher may republish the cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCue209AndAdvanceWorkByte1ScenarioScena0300_801F7E34(void) {
  u8 *work;

  func_8015DF18(0x209);
  work = SPAD_PTR_SLOT(u8, 0x44u);
  work[1]++;
}
