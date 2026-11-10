#include "bof3/bof3.h"

extern void func_801F08AC(void); /* @source 0x801F08AC @kind function */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0224
 * @behavior Invokes the target-local helper at 0x801F08AC, then decrements
 *           work-object byte 9 modulo 256 through the scratchpad pointer cell
 *           at 0x1F800044. When the stored byte is zero, invokes the shared
 *           helper at 0x801E5988. Tests the truncated value without reloading
 *           the byte. Takes no arguments and returns nothing. The handler
 *           table word at 0x801F0CE0 references this entry. Helper roles and
 *           the work byte's role remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelper08ACThenDecrementWorkByte9AndInvokeHelperWhenZeroBmagicMagic12003_801F0224(
    void) {
  func_801F08AC();
  if (--g_battle_work[0x09] == 0) {
    func_801E5988();
  }
}
