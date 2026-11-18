#include "bof3/bof3.h"

extern u8 *boss028CurrentWorkRecordState; /* @source 0x1F800044 @kind data */

/*
 * Code pointer run at 0x800C2184, the handler table this dispatcher indexes:
 * entry 0 = 0x800C18E4, entry 1 = 0x800C1950, entry 2 = 0x800C1A08,
 * entry 3 = 0x800C1A54, entry 4 = 0x800C1AA0, entry 5 = 0x800C1B28,
 * entry 6 = 0x800C1B30, entry 7 = 0x800C1B38. The base sits inside the
 * overlay's pointer run labelled T_800C2180, which the reviewed Splat layout
 * places as one data row, so only the indexed word is named here.
 */
extern void (*D_800C2184[])(void); /* @source 0x800C2184 @kind data */


/* @source 0x800C18A0
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's code pointer run based at 0x800C2184 (0x800C18E4,
 * 0x800C1950, 0x800C1A08, 0x800C1A54, 0x800C1AA0, 0x800C1B28, 0x800C1B30,
 * 0x800C1B38) with no arguments. The original does not bound the index before
 * the scaled table load, and the slots after the index byte, after the table
 * load and in the return delay slot stay nops. It writes no state and returns
 * whatever the called handler leaves in $v0; the 0x18-byte frame only keeps $ra
 * at 0x10 across the call. No jal reaches it: its address appears once in this
 * payload, as the word at 0x800C2164 inside the 0x800C2140 data row, so the
 * main program installs it as a callback. Same shape as the exact sibling
 * dispatchWorkByte2Table21acThenInvokeSharedServiceWhenWorkByte0Bit0Set
 * BossBoss02816_800C1C30, whose body differs only in the scaled table load's low
 * immediate, in the guarded shared-service tail it adds after the call, and in
 * the record reload that tail needs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2Table2184BossBoss02816_800C18A0(void) {
  D_800C2184[boss028CurrentWorkRecordState[2]]();
}
