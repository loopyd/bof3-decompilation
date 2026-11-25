#include "bof3/bof3.h"

extern u8 *boss028CurrentRecordState; /* @source 0x1F800044 @kind data */

/*
 * Code pointer run at 0x800C21AC, the seven-slot work-state table this
 * dispatcher indexes: slot 0 = 0x800C1C9C, slot 1 = 0x800C1D90,
 * slot 2 = 0x800C1E80, slot 3 = 0x800C1F2C, slot 4 = 0x800C2024,
 * slot 5 = 0x800C20C8, slot 6 = 0x800C2120. Its first five words sit inside
 * the 0x800C2180 textbin row, which the reviewed Splat layout places whole, and
 * its last two are the words the layout labels T_800C21C0, so only the
 * indexed word is named here.
 */
extern void (*D_800C21AC[])(void); /* @source 0x800C21AC @kind data */


/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller; this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x800C1C30
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's code pointer run based at 0x800C21AC (0x800C1C9C,
 * 0x800C1D90, 0x800C1E80, 0x800C1F2C, 0x800C2024, 0x800C20C8, 0x800C2120) with
 * no arguments. After that call returns it re-reads the pointer cell and, when
 * the work record's unsigned byte at offset 0 has bit 0 set, runs the shared
 * main-executable service func_8014D290. The original does not bound the index
 * before the scaled table load, and the slots after the index byte, after the
 * table load and in the branch delay slot stay nops. It writes no state; the
 * 0x18-byte frame only keeps $ra at 0x10 across the calls, and this shim returns
 * whatever the guard leaves in $v0. The run word at 0x800C21A4, slot 3 of the
 * code pointer run the byte-indexed dispatcher at 0x800C1AF0 reads from its base
 * 0x800C2198, holds this function's own address. Same shape as the exact sibling
 * dispatchWorkByte2Table27d0ThenInvokeSharedServiceWhenWorkByte0Set
 * BossBoss03316_800C256C, whose body differs only in the scaled table load's low
 * immediate and in the bit-0 mask this span applies to the record byte before
 * the guard branch; the near twin func_800C1FB8 of this payload differs only in
 * that same low immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkStep(
    void) {
  D_800C21AC[boss028CurrentRecordState[2]]();
  if (boss028CurrentRecordState[0] & 1) {
    func_8014D290();
  }
}
