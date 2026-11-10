#include "bof3/bof3.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801F03D8(void); /* @source 0x801F03D8 @kind function */

/* @source 0x801F0310
 * @behavior Reads byte 2 of the shared record pointed to by 0x801EB4E0.
 * When it equals 10, increments byte 2 of the scratchpad work object.
 * Always calls the target-local helper at 0x801F03D8 afterward. Published
 * in the payload callback table at offset 0x68; consumes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByte2Is10ThenInvokeHelperBmagicMagic06403_801F0310(void) {
  if (D_801EB4E0[0x2] == 0xA) {
    g_battle_work[2]++;
  }
  func_801F03D8();
}
