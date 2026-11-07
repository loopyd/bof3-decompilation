#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044. This body reads the cell
 * twice, once for the byte-0x4 advance and again inside the guarded tail, and
 * the original re-materialises the cell address with its own lui / lw before
 * each read, so the cell is reached through a named pointer rather than a
 * shared constant base. This target's map binds the name. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF2D4
 * @behavior Per-frame step handler of this overlay: advances the scratchpad
 *           work object's byte 0x4 by two with a byte-width add (lbu /
 *           addiu 2 / sb) and tests the stored byte truncated to eight bits
 *           (andi $v1,$v1,0xFF / bne), so the truncated register - not a reload
 *           - supplies the compared value. Only when that byte reaches 0x10 does
 *           it re-read the scratchpad pointer cell and increment the work
 *           object's sub-step byte 0x2 with a byte-width add (lbu / addiu 1 /
 *           sb). Takes no arguments, returns nothing and keeps no frame. The
 *           80-byte body is instruction for instruction the exact sibling
 *           add2ToWorkByte4AndAdvanceWorkByte2At10BmagicMagic01303_801EF1A0 of
 *           emi/bmagic/magic013/03 at 0x801EF1A0, and the counter-advance half
 *           of the neighbouring lift
 *           incrementWorkWordCAndAdvanceWorkByte2At2EBmagicMagic12103_801EF324,
 *           which advances word 0xC by one and tests 0x2E instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte4AndAdvanceWorkByte2At10BmagicMagic12103_801EF2D4(void) {
  if ((g_battle_work[4] += 2) == 0x10) {
    g_battle_work[2]++;
  }
}
