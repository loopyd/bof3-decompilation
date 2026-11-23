#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original reaches the cell through a map symbol
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) rather than through a constant
 * address, and it keeps that one base for the guard and the increment. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEE68
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           and compares the work object's unsigned byte at offset 0xB with two
 *           through sltiu, so the guard is taken for every value below two.
 *           Only on that path it increments the work byte at offset 0x1 by one
 *           at byte width (lbu/addiu/sb) through the same base, with no second
 *           load of the cell. The pointer word at 0x801EEC08, the second entry
 *           of the overlay's leading pointer run, names this address. Takes no
 *           arguments, returns nothing and keeps no frame. Its 56 bytes
 *           0x801EEE68..0x801EEEA0 compare equal to the original bytes of the
 *           exact lifts at 0x801EF070 of bmagic/magic088/03 and 0x801EF048 of
 *           bmagic/magic167/03, which supplied the statement order, operand
 *           forms and this behavior name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementByteUnderTwo(void) {
  if (g_battle_work[0xB] < 2u) {
    g_battle_work[1]++;
  }
}
