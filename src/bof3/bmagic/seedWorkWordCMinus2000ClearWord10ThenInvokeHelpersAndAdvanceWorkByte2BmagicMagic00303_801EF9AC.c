#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC */

/* @source 0x801EF9AC
 * @behavior Seeds work word 0xC with -0x2000 and clears word 0x10, then
 *           passes the work pointer to helper 0x801DD350 and calls helper
 *           0x8014DAEC, discarding its result. Reloads the scratchpad work
 *           pointer and increments byte 2. Takes no arguments and returns
 *           nothing. The payload handler table references this entry at
 *           offset 0x20; field roles remain offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWordCMinus2000ClearWord10ThenInvokeHelpersAndAdvanceWorkByte2BmagicMagic00303_801EF9AC(void) {
  u8 *work = g_battle_work;
  u8 *updatedWork;

  *(s32 *)(work + 0xC) = -0x2000;
  *(s32 *)(work + 0x10) = 0;
  func_801DD350(work);
  func_8014DAEC();
  updatedWork = g_battle_work;
  updatedWork[2]++;
}
