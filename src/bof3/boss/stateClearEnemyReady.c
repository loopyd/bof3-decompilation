#include "bof3/bof3.h"

/* Scratchpad pointer cell holding this overlay work record. Declared as the
 * absolute-address symbol so the use reloads the cell with the lui plus offset
 * load form the original shows. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* EXE-side enemy-readiness helper, enemyReadyOrHelper2 of emi/battle/battle/03.
 * It returns an unsigned byte, which this body tests. This target's generated
 * undefined_funcs_auto.txt binds the address. */
extern u8 func_801E3160(void); /* @source 0x801E3160 */

/* @source 0x800C204C
 * @behavior Overlay body whose address is the leading word of this overlay's
 * payload pointer table at 0x800C35D4; the following words of that table form
 * the handler run based at 0x800C35D8 that
 * callStateAdvanceBoss indexes by the
 * work record's state byte +2, but no instruction in this payload loads the
 * 0x800C35D4 word, so its consumer lies outside the overlay image. The body
 * calls the EXE-side enemy-readiness helper 0x801E3160 unconditionally and
 * tests its unsigned byte result: when that byte is nonzero it advances byte +1
 * of the work record published at scratchpad cell 0x1F800044 and clears that
 * record's byte +2, each access dereferencing the cell again so the increment
 * and the clear load the record pointer separately. When the helper reports
 * zero it stores nothing, and both paths join at one epilogue that keeps only
 * $ra in the 0x18-byte frame. The roles of work bytes +1 and +2 beyond those
 * stores and of the readiness helper's returned byte are not established, so
 * they stay offset- and address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateClearEnemyReady(void) {
  if (func_801E3160() != 0) {
    g_battle_work[1]++;
    g_battle_work[2] = 0;
  }
}
