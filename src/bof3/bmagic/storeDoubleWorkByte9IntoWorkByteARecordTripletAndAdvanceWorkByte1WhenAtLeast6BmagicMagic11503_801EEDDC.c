#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-materializes the cell address (lui $aN,0x1F80 / lw $aN,0x44)
 * for each store group, so the cell is reached through this target-map symbol
 * rather than through a constant-address accessor.
 * @source 0x1F800044 @kind data
 */
extern u8 *g_battle_work;

/* Byte 0x2 of the 12-byte effect record selected by work byte 0xA. The record
 * base is 0x80145BD4 and its bytes +0x2/+0x3/+0x4 hold the triplet this handler
 * writes (the battle lifts func_801DF7EC/func_801E38C0 copy the oscillating
 * factor byte into the same three fields). The original reaches the second and
 * third bytes through their own generated symbols at 0x80145BD7 and 0x80145BD8,
 * which the generated undefined_syms_auto.txt of this target already binds.
 * @source 0x80145BD6 @kind table
 */
extern u8 D_80145BD6[];
extern u8 D_80145BD7[];
extern u8 D_80145BD8[];

/* @source 0x801EEDDC
 * @behavior Per-frame effect step: reads the scratchpad work-object pointer cell
 * at 0x1F800044 once per store group and writes the doubled unsigned byte 0x9 of
 * the work object into the +0x2, +0x3 and +0x4 bytes of the 12-byte effect
 * record selected by the work object's unsigned byte 0xA (the index is scaled by
 * the record stride as idx*3*4). Each of the three stores re-reads both the cell
 * and the two work bytes, so the cell address is rebuilt before every store. It
 * then re-reads the cell and the work object's unsigned byte 0x9, and either
 * returns when that byte is below 6 (sltiu $v0,$v0,0x6 against a materialized 6)
 * or increments the work object's byte 0x1 with a byte-width add (lbu, addiu
 * $v0,$v0,1, sb). Leaf with no frame and no saved registers; takes no arguments,
 * calls nothing, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeDoubleWorkByte9IntoWorkByteARecordTripletAndAdvanceWorkByte1WhenAtLeast6BmagicMagic11503_801EEDDC(void) {
  D_80145BD6[g_battle_work[0xA] * 12] = g_battle_work[9] * 2;
  D_80145BD7[g_battle_work[0xA] * 12] = g_battle_work[9] * 2;
  D_80145BD8[g_battle_work[0xA] * 12] = g_battle_work[9] * 2;
  if (g_battle_work[9] >= 6) {
    g_battle_work[1]++;
  }
}
