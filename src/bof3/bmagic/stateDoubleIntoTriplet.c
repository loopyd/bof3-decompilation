#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-materializes the cell address (lui $aN,0x1F80 / lw $aN,0x44)
 * for each store group, so the cell is read once per access.
 * @source 0x1F800044 @kind data
 */
extern u8 *g_battle_work;

/* The 12-byte positional effect records based at 0x80145BD4 carry the triplet
 * this handler writes in their bytes +0x2/+0x3/+0x4, which Splat reaches as the
 * three separate words 0x80145BD6, 0x80145BD7 and 0x80145BD8. Only the base
 * word needs a kind here; this target's generated undefined-symbol table already
 * binds the other two.
 * @source 0x80145BD6 @kind table
 */
extern u8 D_80145BD6[];
extern u8 D_80145BD7[];
extern u8 D_80145BD8[];

/* @source 0x801EEDE0
 * @behavior Per-frame effect step reached from the overlay entry's code pointer
 * run: reads the scratchpad work-object pointer cell at 0x1F800044 once per
 * store group and writes the doubled unsigned work byte 0xB into the
 * +0x2/+0x3/+0x4 triplet bytes of the 12-byte effect record selected by the
 * work object's unsigned byte 0xA (the index is scaled by the record stride as
 * idx*3*4). Each of the three stores re-reads the cell and both work bytes, so
 * the cell address is rebuilt before every store. It then re-reads the cell and
 * the work object's unsigned byte 0xB and, when that byte is exactly 7
 * (bne against a materialized 7), increments the work object's byte 0x1 with a
 * byte-width add (lbu, addiu $v0,$v0,1, sb); otherwise it returns. Leaf with no
 * frame and no saved registers: takes no arguments, calls nothing, returns
 * nothing and reads or writes no other state. Shape evidence: the handler for
 * the same record table in emi/bmagic/magic115/03 doubles and tests work byte 9
 * against a bound of 6 where this span doubles and tests work byte 0xB against
 * an equality with 7; a payload scan found no byte-identical twin of this
 * 208-byte span under out/binaries/emi.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateDoubleIntoTriplet(void) {
  D_80145BD6[g_battle_work[0xA] * 12] = g_battle_work[0xB] * 2;
  D_80145BD7[g_battle_work[0xA] * 12] = g_battle_work[0xB] * 2;
  D_80145BD8[g_battle_work[0xA] * 12] = g_battle_work[0xB] * 2;
  if (g_battle_work[0xB] == 7) {
    g_battle_work[1]++;
  }
}
