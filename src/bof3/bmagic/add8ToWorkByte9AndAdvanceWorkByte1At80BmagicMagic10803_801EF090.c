#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map as
 * g_battle_work. The byte store through the loaded pointer may alias the cell,
 * so the original re-materializes `lui $v1,0x1F80` / `lw $v1,0x44($v1)` for the
 * second access group instead of reusing the first pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF090
 * @behavior Per-frame step handler entered through the six-word code-pointer
 *           run at 0x801EFC9C, which Splat places as this target's `textbin` row
 *           T_801EFC9C: its word at payload offset 0x10A8, entry index 3, holds
 *           0x801EF090, and the only reader of that run is
 *           dispatchWorkByte1HandlerThenDrawTwoMatrixPassesWhenWorkByte0Set
 *           BmagicMagic10803_801EED7C, which scales work byte 0x01 by four and
 *           calls the word, so no jal in the payload names this span and the
 *           dispatcher's own byte-0x01 increment waits on this body. It reads
 *           the work object through the pointer cell at 0x1F800044 and adds 8
 *           to its unsigned byte 9 at byte width (lbu / addiu 8 / sb). It then
 *           re-reads the cell and reloads byte 9 from memory, so the compared
 *           value comes from that second load rather than from the truncation
 *           of a register (no andi), and only when it equals 0x80 does the
 *           reloaded pointer increment work byte 0x01 by one at byte width.
 *           Byte 0x01 is the step index of the handler run this body was chosen
 *           from, so reaching 0x80 advances the overlay's step; byte 9's game
 *           role is not established and stays offset-anchored. Leaf with no
 *           frame, no argument use and no callee; it returns through the jr $ra
 *           at 0x801EF0DC. Statement order, operand forms and declaration style
 *           follow the exact siblings
 *           add4ToWorkByte9AndAdvanceWorkByte2At40BmagicMagic13103_801F053C
 *           (same cell, same byte-9 add, same second cell load and byte-9
 *           reload, same incremented advancing byte; only the added constant,
 *           the compared constant and the advanced offset differ) and
 *           incrementWorkByteAAndAdvanceWorkByte1At60BmagicMagic10803_801EF0E0
 *           (same target, same cell, same work-byte-0x01 advance). No
 *           byte-identical copy of this 0x50-byte span exists in the 396 EMI
 *           payloads under out/binaries/emi.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add8ToWorkByte9AndAdvanceWorkByte1At80BmagicMagic10803_801EF090(void) {
  g_battle_work[9] += 8;
  if (g_battle_work[9] == 0x80) {
    g_battle_work[1]++;
  }
}
