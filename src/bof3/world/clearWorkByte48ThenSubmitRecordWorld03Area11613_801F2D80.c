#include "bof3/bof3.h"

/* Work-record pointer cell in the shared scratchpad. It lies outside every
 * overlay payload and this target's map binds no name for it, so it keeps the
 * raw address spelling the exact lifts of the same cell use. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared submitter in the main executable. */
extern void func_80196718(void *record); /* @source 0x80196718 @kind function */

/* @source 0x801F2D80
 * @behavior Clears byte 0x48 of the work record published through the
 *           scratchpad pointer cell 0x1F800044, then reloads that cell and
 *           passes the record to the shared main-executable submitter
 *           func_80196718. The cell is reloaded because the byte store may
 *           reach the cell word itself. The record pointer is loaded before the
 *           0x18-byte frame is created, and that frame only keeps $ra across
 *           the call, which has a nop delay slot. Nothing in this payload jumps
 *           here: no jal encodes the address, and the word 0x801F2D80 occurs
 *           only once, at 0x801F347C, the tenth entry of the code-pointer run
 *           that begins at 0x801F3458 inside this overlay tail blob and whose
 *           base is named again by the word at 0x801F34B8 - so the entry takes
 *           no arguments and returns nothing. The role of record byte 0x48 is
 *           unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte48ThenSubmitRecordWorld03Area11613_801F2D80(void) {
  D_1F800044[0x48] = 0;
  func_80196718(D_1F800044);
}
