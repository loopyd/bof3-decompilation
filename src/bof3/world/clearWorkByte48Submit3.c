#include "bof3/bof3.h"

/* Work-record pointer cell in the shared scratchpad. It lies outside every
 * overlay payload, and this target's map binds no name for it, so it keeps the
 * raw address spelling. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared submitter in the main executable; it receives the work record this
 * body publishes through the scratchpad pointer cell. Its own role is not
 * established here. */
void func_80196718(void* record); /* @source 0x80196718 */

/* @source 0x801F2C34
 * @behavior Entry 1 of the three-entry handler run at 0x801F3AB0, which the
 *           overlay names through the pointer word at 0x801F3AF8 and whose
 *           word 0 is 0x801F2C04 and word 2 the record-flag clear at
 *           0x801F2C68: clears byte 0x48 of the work record published through
 *           the scratchpad pointer cell 0x1F800044, then reloads that cell and
 *           passes the record to the shared submitter func_80196718. The cell
 *           is read again because the byte store may reach the cell word
 *           itself, so the body reads the cell twice instead of keeping a
 *           local. The record pointer is loaded before the 0x18-byte frame is
 *           created, and that frame only keeps $ra across the call, which runs
 *           with a nop delay slot. It takes no arguments, returns nothing and
 *           calls nothing else. Its only reference in the payload is the run
 *           word at 0x801F3AB4, so the handler is reached from outside the
 *           payload and its argument roles stay structural. The role of record
 *           byte 0x48 is not established, so the name describes the observed
 *           steps.
 *           Clone evidence: a payload cmp measured this fifty-two-byte span at
 *           offset 0x34 word for word equal to the same span in the payloads of
 *           BIN/WORLD00/AREA036#13 (offset 0x30), BIN/WORLD02/AREA100#13
 *           (0x9D4), BIN/WORLD02/AREA112#13 (0xA38), BIN/WORLD01/AREA059#13
 *           (0x15C), BIN/WORLD03/AREA116#13 (0x180), BIN/WORLD03/AREA143#13
 *           (0x1040) and BIN/WORLD03/AREA145#14 (0x1408). Statement order,
 *           operand forms and declaration style are transcribed from the exact
 *           lift clearWorkByte48ThenSubmitRecordWorld00Area03613_801F2C30.
 *           Representation note: the shared-header form
 *           SPAD_PTR_SLOT(u8, 0x44u) measured 9/14 instructions (bytes 52->56)
 *           on the first gate of this lift, because two uses of the same
 *           constant cell address let the compiler keep one 0x1F800044 base in
 *           a register (`lui`/`ori` plus displacement zero), while the original
 *           materializes 0x1F800044 once per read through %hi/%lo of the cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte48Submit(void) {
  D_1F800044[0x48] = 0;
  func_80196718(D_1F800044);
}
