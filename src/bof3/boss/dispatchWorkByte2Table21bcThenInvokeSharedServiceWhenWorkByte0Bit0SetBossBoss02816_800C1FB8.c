#include "bof3/bof3.h"

extern u8 *boss028CurrentWorkRecordState; /* @source 0x1F800044 @kind data */

/*
 * Tail of the overlay's code pointer run, the three words this dispatcher
 * indexes from base 0x800C21BC: 0x800C21BC = 0x800C2024, 0x800C21C0 =
 * 0x800C20C8, 0x800C21C4 = 0x800C2120. The run based at 0x800C2198 that the
 * byte-indexed dispatcher dispatchIndexedHandlerBossBoss02816_800C1AF0 reads
 * starts 0x24 bytes earlier, so this base is that run's word 9, and the payload
 * ends at 0x800C21C8 with these three words. The last two are the words the
 * reviewed Splat layout labels T_800C21C0, so only the indexed word is named
 * here.
 */
extern void (*D_800C21BC[])(void); /* @source 0x800C21BC @kind data */


/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller; this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x800C1FB8
 * @behavior Overlay step dispatch on work-record byte 2 from the tail of this
 * overlay's handler run: loads the work-record pointer published at the
 * scratchpad pointer cell 0x1F800044, takes that record's unsigned byte at
 * offset 2, scales it by four and calls the matching entry of the code pointer
 * run based at 0x800C21BC (0x800C2024, 0x800C20C8, 0x800C2120) with no
 * arguments, so a work byte of 0 through 2 selects the payload's last three
 * handler words. After that call returns it re-reads the pointer cell and, when
 * the work record's unsigned byte at offset 0 has bit 0 set, runs the shared
 * main-executable service func_8014D290. The original does not bound the index
 * before the scaled table load, and the slots after the index byte, after the
 * table load and in the branch delay slot stay nops. It writes no state; the
 * 0x18-byte frame only keeps $ra at 0x10 across the calls, and this shim
 * returns whatever the guard leaves in $v0. The payload word at 0x800C21A8
 * holds this function's own address, word 4 of the run based at 0x800C2198 the
 * byte-indexed dispatcher at 0x800C1AF0 reads. Same C shape as the exact
 * sibling dispatchWorkByte2Table21acThenInvokeSharedServiceWhenWorkByte0Bit0Set
 * BossBoss02816_800C1C30, whose 0x6C-byte span at 0x800C1C30 was compared word
 * by word with this one: the only differing word was the scaled table load's
 * low immediate, 0x800C21AC there against 0x800C21BC here, and every relative
 * branch and the shared service call word are equal. The sibling
 * dispatchWorkByte2Table27d0ThenInvokeSharedServiceWhenWorkByte0Set
 * BossBoss03316_800C256C keeps the same order with a non-zero guard test.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2Table21bcThenInvokeSharedServiceWhenWorkByte0Bit0SetBossBoss02816_800C1FB8(
    void) {
  D_800C21BC[boss028CurrentWorkRecordState[2]]();
  if (boss028CurrentWorkRecordState[0] & 1) {
    func_8014D290();
  }
}
