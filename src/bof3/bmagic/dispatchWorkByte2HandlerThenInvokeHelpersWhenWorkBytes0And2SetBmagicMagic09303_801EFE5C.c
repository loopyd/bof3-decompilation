#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F08C4[])(void); /* @source 0x801F08C4 @kind data */

extern void func_800AF764(void); /* @source 0x800AF764 @kind function */
extern void func_801F01F0(void); /* @source 0x801F01F0 @kind function */

/* @source 0x801EFE5C
 * @behavior Calls the handler indexed by work-record byte 2 in the table at
 *   0x801F08C4, with no index bound. The record pointer comes from scratchpad
 *   cell 0x1F800044. After dispatch, re-reads that cell and tests record bytes
 *   0 and 2 in order. When both are nonzero, calls 0x800AF764 followed by the
 *   local rendering routine at 0x801F01F0. All calls pass no arguments and
 *   their results are unused; this function returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeHelpersWhenWorkBytes0And2SetBmagicMagic09303_801EFE5C(void) {
  D_801F08C4[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    func_800AF764();
    func_801F01F0();
  }
}
