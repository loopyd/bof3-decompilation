#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF334
 * @behavior Increments scratchpad work-object byte 0x09, reloads the work
 * pointer and increments byte 0x0A, then reloads it again and increments byte
 * 0x02 only when byte 0x09 equals 0x10. The byte stores may alias the pointer
 * cell, so each following access group reads the cell again. A pointer in the
 * overlay's dispatch data at 0x801F0360 names this entry. Takes no arguments
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkBytes9AThenAdvanceWorkByte2At10BmagicMagic08603_801EF334(void) {
  u8 *work9;
  u8 *workA;
  u8 *state;

  work9 = g_battle_work;
  work9[9]++;
  workA = g_battle_work;
  workA[0xA]++;
  state = g_battle_work;
  if (state[9] == 0x10) {
    state[2]++;
  }
}
