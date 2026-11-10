#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F04EC[])(void); /* @source 0x801F04EC @kind data */
extern void func_800AF764(void); /* @source 0x800AF764 @kind function */
extern void func_801EF818(void); /* @source 0x801EF818 @kind function */

/* @source 0x801EF554
 * @behavior Calls the handler indexed by unsigned work byte 2 in the local
 *   table at 0x801F04EC, without bounding the index. Re-reads the scratchpad
 *   work-record pointer cell at 0x1F800044 after the handler returns. Only when
 *   work bytes 0 and 2 are both nonzero, tested in that order, calls helpers
 *   0x800AF764 and 0x801EF818 in that order. All calls take no arguments and
 *   their results are unused. This entry takes no arguments and returns
 *   nothing; its address occupies the table word at 0x801F04E4. The helper
 *   roles and work fields remain address- or offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeHelpersWhenWorkBytes0And2SetBmagicMagic14403_801EF554(void) {
  D_801F04EC[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    func_800AF764();
    func_801EF818();
  }
}
