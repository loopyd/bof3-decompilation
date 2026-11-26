#include "bof3/emi/dispatchers.h"

/*
 * Handler run of this overlay, based at 0x800C19A4: fifteen words, its word 0
 * being this payload's step-setup publisher 0x800C1848, words 1 through 11
 * main-executable handlers (0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980,
 * 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8,
 * 0x801E5288) and words 12 through 14 this overlay's inert handlers
 * 0x800C18E4, 0x800C18EC and 0x800C18F4. The run begins 0x14 bytes into the
 * reviewed `textbin` row bound at 0x800C1990 and ends where the payload ends,
 * so that row stays one row and this declaration binds only the word the
 * scaled index is added to. The generated undefined_syms_auto.txt row for
 * D_800C19A4 already covers the address, so this declaration needs no map row.
 */
extern void (*D_800C19A4[])(void); /* @source 0x800C19A4 @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch: loads the work-record pointer published at
 * the scratchpad pointer cell 0x1F800044 once, before the frame is created,
 * takes that record's unsigned byte at offset 0x01 as the index, scales it by
 * four and calls the handler pointer at that index of the local handler run
 * based at 0x800C19A4 with no arguments. The index is not bounded before the
 * scaled table load. The chosen handler's return value survives the epilogue,
 * which only reloads $ra from 0x10 of the 0x18-byte frame, so this shim keeps
 * whatever the handler leaves in $v0. It reads no other state and writes none.
 * No instruction in this payload names its address and no word of the handler
 * run holds it, so the overlay loader that steps the run is its caller: the
 * argument list is empty. This is the body the shared EMI dispatcher macro
 * emits, and the exact sibling dispatchStateByte1Table1aacBossBoss01716_800C1804
 * of BIN/BOSS/BOSS017.EMI#16 supplied its statement order; the table base and
 * the record byte are this payload's own. The 68-byte span is also
 * byte-identical to the one at offset 4 of BIN/BOSS/BOSS042.EMI#16 (proved with
 * cmp), whose own Splat row was still `asm` when this one was lifted, so there
 * was no source to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchWorkByte1Table, dispatchWorkByte1Table, D_800C19A4);
