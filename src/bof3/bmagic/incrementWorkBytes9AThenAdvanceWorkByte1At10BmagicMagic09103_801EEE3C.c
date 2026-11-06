#include "bof3/bof3.h"

/* The statement order, the per-group scratchpad reloads and the declaration
 * style follow the exact sibling
 * addWorkMotionPairAndIncrementBytes9AThenAdvanceWorkByte2At10
 * (0x801EF55C, magic043/03), whose tail increments work byte 0x09, reloads the
 * cell, increments work byte 0x0A, reloads the cell again and tests the
 * reloaded byte against 0x10 before advancing a work byte. Each group loads the
 * cell afresh because the preceding store through the loaded pointer may alias
 * the cell, and the guard's pointer stays in one register (0x1F800044 loads
 * into $a0 at 0x801EEE74) for the increment in its arm. */

extern u8 * volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEE3C
 * @behavior Increments scratchpad work-object byte 0x09 by one, re-reads the
 * work pointer cell at 0x1F800044 and increments byte 0x0A by one, then
 * re-reads the cell once more and increments work byte 0x01 by one only when
 * work byte 0x09 equals 0x10. The pointer cell is loaded three times, byte
 * 0x09 is tested with a zero-extended load and the constant 0x10 is
 * materialized after that load and ahead of the guard branch. Leaf with no
 * frame; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkBytes9AThenAdvanceWorkByte1At10BmagicMagic09103_801EEE3C(void) {
  u8 *work9;
  u8 *workA;
  u8 *state;

  work9 = g_battle_work;
  work9[9]++;
  workA = g_battle_work;
  workA[0xA]++;
  state = g_battle_work;
  if (state[9] == 0x10) {
    state[1]++;
  }
}
