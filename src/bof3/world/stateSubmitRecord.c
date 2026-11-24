#include "bof3/bof3.h"

/* Work-record pointer cell in the shared scratchpad. It lies outside every
 * overlay payload, and this target's map binds no name for it, so it keeps the
 * raw address spelling its sibling sources use. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared submitter in the main executable. */
extern void func_80196718(void *record); /* @source 0x80196718 @kind function */

/* @source 0x801F2D5C
 * @behavior Clears byte 0x48 of the work record published through scratchpad
 *           pointer cell 0x1F800044, then reloads that cell and passes the
 *           record to the shared submitter func_80196718. The cell is reloaded
 *           because the byte store may reach the cell word itself. The record
 *           pointer is loaded before the 0x18-byte frame is created, and that
 *           frame only keeps $ra across the call, which has a nop delay slot.
 *           No jal in the payload names this address: the only word holding it
 *           is 0x801F37A8, entry 1 of the pointer run based at 0x801F37A4,
 *           whose base appears in the words at 0x801F37E8 and 0x801F381C,
 *           beside the neighbouring entries 0x801F2D30, 0x801F2D90 and
 *           0x801F2E08. The entry therefore takes no arguments, returns
 *           nothing and reads no other state. The role of record byte 0x48 is
 *           unknown.
 *
 *           Clone evidence: this fifty-two-byte span compared equal word for
 *           word against the same span in the payloads of
 *           BIN/WORLD02/AREA100.EMI#13 (offset 0x9D4), BIN/WORLD02/AREA112.EMI#13
 *           (0xA38), BIN/WORLD03/AREA116.EMI#13 (0x180),
 *           BIN/WORLD03/AREA146.EMI#13 (0x34), BIN/WORLD03/AREA143.EMI#13
 *           (0x1040) and BIN/WORLD03/AREA145.EMI#14 (0x1408). The clone adapter
 *           was not issued: the body carries this target's own pointer-run facts
 *           and the scratchpad cell spelling, so the hand copy is the route.
 *           Statement order, operand forms and declaration style are
 *           transcribed from clearWorkByte48ThenSubmitRecordWorld02Area11213_801F3638,
 *           whose own lift measured exact.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSubmitRecord(void) {
  D_1F800044[0x48] = 0;
  func_80196718(D_1F800044);
}
