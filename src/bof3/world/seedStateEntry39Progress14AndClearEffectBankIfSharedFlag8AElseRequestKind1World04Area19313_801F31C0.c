#include "bof3/bof3.h"

/* @source 0x80144F28 @kind unknown */
extern s32 D_80144F28;
extern u8 D_801448EB;
extern u8 D_801448EC;
extern u8 D_8014832E;
extern u8 D_80143F1D;

s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801F31C0
 * @behavior Probes flag 0x8A of the main-RAM flag bank 0x80144F28 through the
 *           shared flag tester func_8015B5D4. While the probe reports the flag
 *           set, seeds the front-end state-entry tuple with the state selector
 *           0x801448EB = 0x39 and the progress byte 0x801448EC = 0x14 and
 *           clears the effect bank 0x8014832E; when the flag is clear it writes
 *           1 into the request kind byte 0x80143F1D instead. The 0x39 constant
 *           fills the `beqz` delay slot, 0x14 is loaded ahead of the effect
 *           bank store, and the arm closes with a jump to the shared epilogue.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedStateEntry39Progress14AndClearEffectBankIfSharedFlag8AElseRequestKind1World04Area19313_801F31C0(
    void) {
  if (func_8015B5D4((u32)&D_80144F28, 0x8A) != 0) {
    D_801448EB = 0x39;
    D_8014832E = 0;
    D_801448EC = 0x14;
  } else {
    D_80143F1D = 1;
  }
}
