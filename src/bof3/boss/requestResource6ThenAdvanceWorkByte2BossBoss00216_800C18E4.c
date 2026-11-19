#include "bof3/bof3.h"

/* Work-object pointer cell in scratchpad; the original reads the cell once, in
 * the statement after the shared helper returns, and touches only byte 2 of the
 * record it points at. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-program resource request helper: the original passes the
 * resource id 6 and drops the result, so only its argument shape is known
 * here. */
void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* Entry 0 of this overlay's second handler run: the twelve words based at
 * 0x800C1FEC hold in-image code pointers of this payload's functions and shared
 * main-executable handlers, and they are the payload's reviewed `textbin` row
 * D_800C1F9C. The dispatcher at 0x800C18A0 indexes that run from work-record
 * byte 2 and calls the selected entry with no arguments, and the only payload
 * word that names this body is the run word 0x800C1FEC itself, so the
 * parameters stay structural.
 *
 * @source 0x800C18E4
 * @behavior Step 0 of this overlay's second dispatcher run: calls the shared
 * resource request helper at 0x8014D6B8 with 6, then loads the work-object
 * pointer published at the scratchpad cell 0x1F800044 once and increments the
 * unsigned byte at offset 0x02 of that object, the byte the dispatcher at
 * 0x800C18A0 indexes the same run with, so the next dispatch reaches entry 1 at
 * 0x800C191C. The record pointer load sits after the helper call and the 0x18
 * byte frame only keeps $ra at 0x10 across it. Same shape as the already-exact
 * sibling clearWorkBytesBAnd9Invoke8014D6B8With3ThenAdvanceWorkByte2 of
 * BIN/BMAGIC/MAGIC067.EMI#03 at 0x801F03EC, which requests resource 3 and
 * carries two extra leading stores this body does not have.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResource6ThenAdvanceWorkByte2BossBoss00216_800C18E4(void) {
  func_8014D6B8(6);
  D_1F800044[2]++;
}
