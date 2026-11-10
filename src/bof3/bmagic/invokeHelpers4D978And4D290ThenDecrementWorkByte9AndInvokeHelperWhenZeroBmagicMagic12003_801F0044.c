#include "bof3/bof3.h"

extern u8 func_8014D978(void); /* @source 0x8014D978 @kind function */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0044
 * @behavior Invokes the shared helpers at 0x8014D978 and 0x8014D290 in order,
 *           discarding their results, then decrements work-object byte 9
 *           modulo 256 through the scratchpad pointer cell at 0x1F800044.
 *           When the stored byte is zero, invokes the shared helper at
 *           0x801E5988. Tests the truncated value without reloading the byte.
 *           Takes no arguments and returns nothing. The handler table word
 *           at 0x801F0CCC references this entry. Helper roles and the work
 *           byte's role remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelpers4D978And4D290ThenDecrementWorkByte9AndInvokeHelperWhenZeroBmagicMagic12003_801F0044(
    void) {
  func_8014D978();
  func_8014D290();
  if (--g_battle_work[0x09] == 0) {
    func_801E5988();
  }
}
