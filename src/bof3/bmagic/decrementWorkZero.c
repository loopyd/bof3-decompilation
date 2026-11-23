#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EFFB0
 * @behavior Decrements work byte 9 at byte width, subtracts 5 from work word
 * 0x20 and adds 2 to work word 0x14. Then, when the decremented byte is zero,
 * calls the shared helper at 0x801E5988. The pointer word at payload
 * offset 0x2638 names this handler. Takes no arguments and returns nothing;
 * the helper keeps its address-based name and field roles stay offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkZero(void) {
  u32 *work;
  u8 remaining;

  work = (u32 *)g_battle_work;
  remaining = --((u8 *)work)[9];
  work[8] -= 5;
  work[5] += 2;
  if (remaining == 0) {
    func_801E5988();
  }
}
