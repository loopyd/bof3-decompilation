#include "bof3/emi/dispatchers.h"

/*
 * Handler run of this overlay, based at 0x800C1AAC: its word 0 is 0x800C1848,
 * words 1 through 5 and 7 through 11 and 13 through 17 are main-executable
 * handlers, word 6 is 0x800C18A0, word 16 is 0x800C18E4 and words 18 through
 * 20 are the inert handlers 0x800C199C, 0x800C19A4 and 0x800C19AC. The run
 * sits inside the reviewed textbin row bound at 0x800C1A70, so it stays one
 * row and this declaration binds only the word the scaled index is added to.
 * The generated undefined_syms_auto.txt row for D_800C1AAC already covers the
 * address, so this declaration needs no map row.
 */
extern void (*D_800C1AAC[])(void); /* @source 0x800C1AAC @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch: loads the work-record pointer published at
 * the scratchpad pointer cell 0x1F800044 once, before the frame is created,
 * takes that record's unsigned byte at offset 0x01 as the index, scales it by
 * four and calls the handler pointer at that index of the local handler run
 * based at 0x800C1AAC with no arguments. The index is not bounded before the
 * scaled table load. The chosen handler's return value survives the epilogue,
 * which only reloads $ra from 0x10 of the 0x18-byte frame, so this shim keeps
 * whatever the handler leaves in $v0. It reads no other state and writes none.
 * No instruction in this payload names its address and no word of it appears
 * in the overlay handler run, so the loader that steps the run is its caller:
 * the argument list is empty. Same body as the exact sibling
 * dispatchStateByte1Table2660BossBoss00816_800C1804 of BIN/BOSS/BOSS008.EMI#16,
 * which reads the same record byte through a different table base.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callByte1Table1aac, callByte1Table1aac, D_800C1AAC);
