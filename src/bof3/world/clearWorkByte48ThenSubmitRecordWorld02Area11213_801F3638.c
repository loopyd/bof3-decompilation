#include "bof3/bof3.h"

/* Work-record pointer cell in the shared scratchpad. It lies outside every
 * overlay payload, and this target's map binds no name for it, so it keeps the
 * raw address spelling its sibling source in this target uses. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared submitter in the main executable. */
extern void func_80196718(void *record); /* @source 0x80196718 */

/* @source 0x801F3638
 * @behavior Clears byte 0x48 of the work record published through scratchpad
 *           pointer cell 0x1F800044, then reloads that cell and passes the
 *           record to the shared submitter func_80196718. The cell is reloaded
 *           because the byte store may reach the cell word itself. The record
 *           pointer is loaded before the 0x18-byte frame is created, and that
 *           frame only keeps $ra across the call, which has a nop delay slot.
 *           The payload's pointer run D_801F41F8 records this handler's address
 *           at 0x801F48A4. The body takes no arguments, returns nothing and
 *           reads no other state; the role of record byte 0x48 is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte48ThenSubmitRecordWorld02Area11213_801F3638(void) {
  D_1F800044[0x48] = 0;
  func_80196718(D_1F800044);
}
