#include "bof3/emi/dispatchers.h"

/* This overlay's second handler table: the run of code pointers inside the
 * reviewed `textbin` row D_800C3174, which places the words, so the table is
 * only referenced here. Entry 0 is this overlay's 0x800C1DCC (installs the
 * overlay's hooks and work bytes), entries 2, 5, 6, 12 and 13 are this
 * overlay's 0x800C1E34, 0x800C1EC8, 0x800C2030, 0x800C1F0C and 0x800C1F90, and
 * entries 1, 3, 4, 7, 8, 9, 10, 11, 14 and 15 are shared main-executable
 * handlers (0x801E52E8 twice, 0x801E3638, 0x801E3980, 0x801E4AE8, 0x801E4D8C,
 * 0x801E4F64, 0x801E5288, 0x801E3BAC, 0x801E3BD0). */
extern void (*D_800C3244[])(void); /* @source 0x800C3244 @kind data */

/* @source 0x800C1D88
 * @behavior Overlay state dispatch on work-record byte 1: loads the
 *           work-record pointer published at the scratchpad pointer cell
 *           0x1F800044, takes that record's unsigned byte at offset 1, scales
 *           it by four and calls the matching entry of this overlay's handler
 *           table based at 0x800C3244 with no arguments. The original does not
 *           bound the index before the scaled load. It reads no other state
 *           and writes none; the pointer-cell load precedes frame creation,
 *           the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 *           call, and the slot after the index byte and the slot after the
 *           table load stay nops. No in-image reference to this address
 *           exists in the payload: no word equals 0x800C1D88, no lui/addiu
 *           pair forms it and no `jal` targets it, so it can only be entered
 *           from outside the overlay. Same body as the sibling dispatcher at
 *           0x800C1B6C of this overlay; only the table address in the load
 *           word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table3244BossBoss02513_800C1D88, D_800C3244);
