#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. This target's own map row g_battle_work binds 0x1F800044, so the
 * cell keeps that name here; its symbolic address folds into each load
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) and the original re-reads the cell for
 * the byte-0x2 group because the byte store that precedes it goes through the
 * pointer the cell supplied. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF784
 * @behavior Per-step handler of the overlay's work-object state machine. It
 *           reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           (lui $v0,0x1F80 / lw $v0,0x44($v0), empty load-delay slot),
 *           advances that object's unsigned byte 0x9 by one at byte width
 *           (lbu $v1,0x9($v0) / addiu $v1,$v1,1 / sb $v1,0x9($v0)) and tests
 *           the truncated value it just stored (andi $v1,$v1,0xFF, no second
 *           load of the cell) against 0x10 for equality (addiu
 *           $v0,$zero,0x10 / bne with an empty delay slot). Only when that
 *           byte equals 0x10 does it re-materialize the cell with its own
 *           lui/lw pair and increment the object's byte 0x2 at byte width
 *           (lbu $v0,0x2($v1) / addiu $v0,$v0,1 / sb $v0,0x2($v1)),
 *           advancing the byte-0x2 step index the overlay's dispatcher reads.
 *           The 0x50-byte body has no frame, no call and no other store, so it
 *           is a leaf: jr $ra at 0x801EF7CC with an empty delay slot, and no
 *           jal in this 6032-byte payload reaches 0x801EF784. Its address is
 *           instead word 0 of the pointer run workByte2HandlerTableB at
 *           0x801F0384, whose remaining entries name 0x801EF7D4, 0x801EF850,
 *           0x801EF8A0, 0x801EF8EC and 0x801EF974, so that table proves this
 *           function-entry boundary; the table carries no prototype, so the
 *           argument roles stay structural. The roles of work-object bytes 9
 *           and 2 are not established, so both stay offset-anchored. Bytes
 *           0x801EF784..0x801EF7D3 are byte-identical (80 bytes, one payload
 *           scan of every file under out/binaries) to seven already-exact
 *           lifts - emi/bmagic/magic062/03@0x801EF588,
 *           magic067/03@0x801EFFCC, magic079/03@0x801EFCDC and @0x801EFF08,
 *           magic082/03@0x801EF630, magic088/03@0x801EF59C and
 *           magic137/03@0x801EFD00 - which agree on this body's statement
 *           order and on the name incrementWorkByte9AndAdvanceWorkByte2At10;
 *           the statement form below follows the magic137/03 lift measured
 *           exact on those same 80 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte2At10BmagicMagic08603_801EF784(void) {
  if ((g_battle_work[9] += 1) == 0x10) {
    g_battle_work[2]++;
  }
}
