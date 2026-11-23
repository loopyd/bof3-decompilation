#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is a named cell
 * symbol, the form this target's exact siblings use: the symbolic address lets
 * the assembler fold it into each load (lui $vN,0x1F80 / lw $vN,68($vN)). The
 * byte-9 store goes through the first loaded pointer, so the original
 * materialises the cell a second time for the test and reuses that reload for
 * the byte-1 test and the byte-2 increment. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF654
 * @behavior Overlay step handler, entry 9 of the fifteen in-image words of the
 *           handler-pointer run based at 0x801F04D0: the word at 0x801F04F0 is
 *           the only word in this payload that holds this address, so no jal
 *           names the body and the table row proves the entry boundary and the
 *           no-argument ABI. The dispatcher at 0x801EEE9C indexes that run by
 *           work-record byte 1. It reads the scratchpad work-object pointer cell
 *           at 0x1F800044 (lui / lw) and adds four to work-object byte 9 modulo
 *           256 (lbu / addiu 4 / sb). It then materialises the same cell again
 *           with its own lui / lw pair, re-reads the byte it just stored through
 *           that reload, and only when that byte equals 0x11 increments
 *           work-object byte 2 modulo 256 (lbu / addiu 1 / sb), advancing to the
 *           next byte-2 handler step. Leaf with no frame and no saved registers:
 *           it takes no arguments, returns nothing through the jr $ra at
 *           0x801EF69C, and reads or writes no other state. The roles of
 *           work-object bytes 9 and 2 are not established and stay
 *           offset-anchored.
 *
 *           The 0x50-byte span is byte-identical to the reviewed exact lift
 *           add4ToWorkByte9AndAdvanceWorkByte2At11BmagicMagic09203_801EF940 of
 *           emi/bmagic/magic092/03: a payload cmp of the two spans reports zero
 *           differing bytes over 80 bytes, and the only address constant either
 *           body carries is the shared scratchpad cell, so there is no
 *           relocation to re-derive. The statement order, the second cell
 *           materialisation, the reload discipline and the register choice come
 *           from that twin; its name was carried with its own target suffix on
 *           the same evidenced role. The 4 and 0x11 literals are this span's
 *           own bytes. The loader clone generated the raw D_1F800044 spelling;
 *           the named cell is kept because this target's map already binds the
 *           word and this target's exact siblings measure with it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void workByte9AdvanceAt11(void) {
  g_battle_work[9] += 4;
  if (g_battle_work[9] == 0x11) {
    g_battle_work[2]++;
  }
}
