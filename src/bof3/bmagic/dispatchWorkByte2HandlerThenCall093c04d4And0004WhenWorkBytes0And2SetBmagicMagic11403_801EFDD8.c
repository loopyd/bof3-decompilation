#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F1230[])(void); /* @source 0x801F1230 @kind data */

extern void func_801F093C(void); /* @source 0x801F093C @kind function */
extern void func_801F04D4(s32 mode); /* @source 0x801F04D4 @kind function */
extern void func_801F0004(void); /* @source 0x801F0004 @kind function */

/* @source 0x801EFDD8
 * @behavior Calls the handler indexed by unsigned work-record byte 2 in the
 *   table at 0x801F1230, with no index bound. The record pointer comes from
 *   scratchpad cell 0x1F800044. After dispatch, reads that cell again and tests
 *   record bytes 0 and 2 in order. When both are nonzero, calls 0x801F093C,
 *   reads the cell again, passes unsigned record byte 0xB to 0x801F04D4,
 *   then calls 0x801F0004. Other calls take no arguments, their results are
 *   unused, and this function returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenCall093c04d4And0004WhenWorkBytes0And2SetBmagicMagic11403_801EFDD8(void) {
  D_801F1230[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    func_801F093C();
    func_801F04D4(g_battle_work[0xB]);
    func_801F0004();
  }
}
