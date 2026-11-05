#include "bof3/bof3.h"

extern u8 D_801448EB;
extern u8 D_801448EC;
extern u16 D_801448EE;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;
extern u8 D_80147ACF;
extern u16 D_80147AD6;

void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801FB168
 * @behavior Overlay frame handler and entry 2 of the handler-pointer table at
 * D_801FE970, which dispatchProgressHandlerScenarioScena0800_801FAB5C indexes
 * with the signed shared byte D_80146874; it drives the shared sub-state byte
 * D_80146875 through a three-step sequence. Sub-state 0 stores 3 in
 * D_80147ACF, clears the shared halfword D_80147AD6 and advances to 1.
 * Sub-state 1 waits until the shared counter byte D_80146867 reads 0x12, then
 * arms the shared countdown halfword D_80146876 with 0x10 and advances to 2.
 * Sub-state 2 post-decrements that countdown and, when the pre-decrement value
 * reads 0, sets scenario flag bit 3 through func_8015B580(D_8014686C, 3), seeds
 * D_801448EE = 1, D_801448EB = 5 and D_801448EC = 3, and returns the overlay to
 * handler 0 by clearing D_80146874 together with the sub-state byte. Any other
 * sub-state value falls through to the exit without touching memory. It reads
 * and writes no other memory and returns nothing; the 0x18-byte frame only
 * keeps $ra across func_8015B580. D_80146875 is reached through one byte
 * pointer, so the initial load and the three stores share the base the
 * prologue leaves in $s0, while every other cell materialises its own
 * 0x8014/0x8015 page in $at per access.
 *
 * Name evidence: entry 2 of the table at D_801FE970 is this address (the word
 * at 0x801FE978 reads 0x801FB168) and dispatchProgressHandlerScenarioScena0800_801FAB5C
 * selects it from D_80146874, so the function runs as that byte's value-2
 * handler. "waitForSharedCounter" is the accepted wording of the exact
 * func_801F394C lift in world/area027 for the same D_80146867 test ("waits for
 * the shared counter byte at 0x80146867 to read"), "ArmCountdown" covers
 * D_80146876 = 0x10, "SetFlagBit3" covers the func_8015B580(D_8014686C, 3) call
 * that the exact scena08 siblings describe as a scenario flag bit, and
 * "ResetHandler" covers the closing D_80146874 = 0 that reselects entry 0
 * (noopHandlerScenarioScena0800_801FAB98) plus the D_80146875 clear.
 * @status exact
 * @match 100.00
 * @residual none
 */
void waitForSharedCounterArmCountdownThenSetFlagBit3AndResetHandlerScenarioScena0800_801FB168(void) {
  u8* state;

  state = &D_80146875;
  switch (*state) {
    case 0:
      D_80147ACF = 3;
      D_80147AD6 = 0;
      *state = 1;
      break;

    case 1:
      if (D_80146867 == 0x12) {
        D_80146876 = 0x10;
        *state = 2;
      }
      break;

    case 2:
      if (D_80146876-- == 0) {
        func_8015B580(D_8014686C, 3);
        D_801448EE = 1;
        D_801448EB = 5;
        D_801448EC = 3;
        D_80146874 = 0;
        *state = 0;
      }
      break;
  }
}
