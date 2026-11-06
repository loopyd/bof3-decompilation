#include "bof3/emi/dispatchers.h"

/* This overlay's handler table: the run of code pointers starting at
 * 0x800C31F0 and reaching the reviewed `textbin` row D_800C3174, which places
 * the words, so the table is only referenced here. Entry 0 is this overlay's
 * 0x800C1BB0, entries 1 through 5 are shared main-executable handlers
 * (0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00), entry 6 is
 * this overlay's 0x800C1C6C, entry 16 is 0x800C1CB0, and the words from
 * 0x800C3238 on belong to the table the sibling dispatcher at 0x800C1D2C
 * indexes. */
extern void (*D_800C31F0[])(void); /* @source 0x800C31F0 @kind data */

/* @source 0x800C1B6C
 * @behavior Overlay state dispatch on work-record byte 1: loads the
 *           work-record pointer published at the scratchpad pointer cell
 *           0x1F800044, takes that record's unsigned byte at offset 1, scales
 *           it by four and calls the matching entry of this overlay's handler
 *           table based at 0x800C31F0 with no arguments. The original does not
 *           bound the index before the scaled load. It reads no other state
 *           and writes none; the pointer-cell load precedes frame creation,
 *           the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 *           call, and the slot after the index byte and the slot after the
 *           table load stay nops. No in-image reference to this address
 *           exists in the payload: no word equals 0x800C1B6C and no lui/addiu
 *           pair forms it, so neither a `jal` nor a table entry reaches it and
 *           it can only be entered from outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table31f0BossBoss02513_800C1B6C, D_800C31F0);
