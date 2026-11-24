#include "bof3/bof3.h"

/* Work-record pointer cell in the shared scratchpad. It lies outside every
 * overlay payload, and this target's map binds no name for it, so it keeps the
 * raw address spelling this target's sibling source uses. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared submitter in the main executable. */
extern void func_80196718(void *record); /* @source 0x80196718 @kind function */

/* @source 0x801F2C30
 * @behavior Clears byte 0x48 of the work record published through scratchpad
 *           pointer cell 0x1F800044, then reloads that cell and passes the
 *           record to the shared submitter func_80196718. The cell is reloaded
 *           because the byte store may reach the cell word itself. The record
 *           pointer is loaded before the 0x18-byte frame is created, and that
 *           frame only keeps $ra across the call, which has a nop delay slot.
 *           The payload records this handler's address at 0x801F32BC, the
 *           second word of the two-entry run based at 0x801F32B8, whose base
 *           address appears in the words at 0x801F32C8 and 0x801F32FC, so the
 *           entry takes no arguments and returns nothing. The role of record
 *           byte 0x48 is unknown.
 *           Clone evidence: this thirty-four-byte span compared equal word for
 *           word against the same span in the payloads of
 *           BIN/WORLD02/AREA100#13 (offset 0x9D4), BIN/WORLD02/AREA112#13
 *           (0xA38), BIN/WORLD01/AREA059#13 (0x15C), BIN/WORLD03/AREA116#13
 *           (0x180), BIN/WORLD03/AREA146#13 (0x34), BIN/WORLD03/AREA143#13
 *           (0x1040) and BIN/WORLD03/AREA145#14 (0x1408). The adapter was not
 *           issued: a hand copy is the route because the body carries this
 *           target's own pointer-run facts and the scratchpad cell under the
 *           name this target already uses. Statement order, operand forms and
 *           declaration style are transcribed from
 *           clearWorkByte48ThenSubmitRecordWorld02Area11213_801F3638, whose own
 *           gate measured exact.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateClearByte48Submit(void) {
  D_1F800044[0x48] = 0;
  func_80196718(D_1F800044);
}
