#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */
extern void func_80196718(void *work); /* @source 0x80196718 @kind function */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF9FC
 * @behavior Adds work words 0xC and 0x10 to words 0x34 and 0x38,
 *           respectively, then calls helper 0x8014DAEC. Reloads the work
 *           pointer and, when those words equal words 0x18 and 0x1C,
 *           submits that work record to 0x80196718, decrements shared work
 *           byte 0xB with wraparound and calls helper 0x801E5988.
 *           Takes no arguments and returns nothing. The entry is referenced
 *           at payload offset 0x24; field roles remain offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkWordPairThenSubmitAtTargetDecrementGlobalWorkByteBAndInvokeHelperBmagicMagic00303_801EF9FC(void) {
  u8 *work = g_battle_work;
  u8 *updatedWork;

  *(u32 *)(work + 0x34) += *(u32 *)(work + 0xC);
  *(u32 *)(work + 0x38) += *(u32 *)(work + 0x10);
  func_8014DAEC();
  updatedWork = g_battle_work;
  if (*(u32 *)(updatedWork + 0x34) == *(u32 *)(updatedWork + 0x18) &&
      *(u32 *)(updatedWork + 0x38) == *(u32 *)(updatedWork + 0x1C)) {
    u8 *globalWork;

    func_80196718(updatedWork);
    globalWork = D_801EB4E0;
    globalWork[0xB]--;
    func_801E5988();
  }
}
