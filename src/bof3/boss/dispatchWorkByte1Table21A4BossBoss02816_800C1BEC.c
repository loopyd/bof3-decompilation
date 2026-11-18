#include "bof3/bof3.h"

/*
 * Code pointer run at 0x800C21A4, the seven-slot handler table this dispatcher
 * indexes: slot 0 = 0x800C1C30, slot 1 = 0x800C1FB8, slot 2 = 0x800C1C9C,
 * slot 3 = 0x800C1D90, slot 4 = 0x800C1E80, slot 5 = 0x800C1F2C,
 * slot 6 = 0x800C2024. The base sits inside the overlay's pointer run labelled
 * T_800C2180, which the reviewed Splat layout places as one data row, so only
 * the indexed word is named here.
 */
extern void (*D_800C21A4[])(void); /* @source 0x800C21A4 @kind data */

/* Work-record pointer cell in the scratchpad. The original materialises this
 * cell with its own lui/lw pair, so it is read through the named target symbol
 * instead of the constant-address accessor. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x800C1BEC
 * @behavior Overlay handler dispatch on work-record byte 1: loads the
 * work-record pointer published at the scratchpad pointer cell 0x1F800044,
 * takes that record's unsigned byte at offset 1, scales it by four and calls
 * the matching entry of this overlay's code pointer run based at 0x800C21A4
 * (0x800C1C30, 0x800C1FB8, 0x800C1C9C, 0x800C1D90, 0x800C1E80, 0x800C1F2C,
 * 0x800C2024) with no arguments. The original does not bound the index before
 * the scaled table load, and the slots after the index byte, after the table
 * load and in the return delay slot stay nops. It writes no state and returns
 * whatever the called handler leaves in $v0; the 0x18-byte frame only keeps $ra
 * at 0x10 across the call. No jal reaches it: the word at 0x800C21A4, slot 0 of
 * this overlay's own handler run, holds its address, so the byte-2 step
 * dispatchers select it as a handler. Same shape as the exact sibling
 * dispatchWorkByte2Table2184BossBoss02816_800C18A0, whose body differs only in
 * the work-record byte it reads and in the scaled table load's low immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Table21A4BossBoss02816_800C1BEC(void) {
  D_800C21A4[D_1F800044[1]]();
}
