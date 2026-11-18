#include "bof3/bof3.h"

/* Shared front-end phase byte of the main executable: sub-state 0 latches this
 * byte to 2 and sub-state 1 holds until it stops reading 2, which is the
 * store-then-wait use the scenario overlay sequences make of the cell.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Shared primary scenario state byte of the main executable: the closing store
 * of 0 ends the chain this handler belongs to.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;

/* Shared scenario sub-state byte of the main executable and the switch selector
 * of this body.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

/* Shared front-end action helper of the main executable, outside this payload:
 * it starts the action its argument ids.
 * @source 0x80150224 @kind function
 */
extern void func_80150224(s32 arg0);
/* Shared front-end release helper of the main executable, outside this payload.
 * @source 0x8015C058 @kind function
 */
extern void func_8015C058(void);

/* @source 0x801FCE30
 * @behavior Two-step handler over the shared sub-state byte D_80146875
 * (0x80146875): it takes that byte's address once into a local pointer, reads it
 * with `lbu` once as the switch selector and runs one of two sequences over it.
 * Sub-state 0 starts action 0x34 through the shared action helper func_80150224
 * (0x80150224), latches the shared phase byte D_80143BB0 (0x80143BB0) to 2 and
 * advances the sub-state byte to 1. Sub-state 1 returns while that phase byte
 * still reads 2 and, once it differs, runs the shared front-end release helper
 * func_8015C058 (0x8015C058), clears the sub-state byte and stores 0 in the
 * shared primary state byte D_80146874 (0x80146874). Any other sub-state value
 * returns without touching any byte. It takes no arguments and returns nothing,
 * and its 0x18-byte frame keeps $ra and $s0, the latter because the pointer to
 * the sub-state byte stays live across both calls; the `lui`/`addiu` pair that
 * forms that pointer is scheduled between the two prologue saves. The payload
 * refers to 0x801FCE30 through exactly one word, the pointer at 0x801FE300: no
 * `jal` or `j` targets it, and that word is word 25 of the
 * contiguous code-pointer run based at 0x801FE29C that the exact dispatcher
 * dispatchProgressHandlerScenarioScena0100_801F7FC4 (0x801F7FC4) indexes with
 * the shared scenario progress byte D_80146872. Its own bytes have no
 * byte-identical copy at any other offset of the 396 EMI payloads under
 * `out/binaries/emi`; the closest windows are four relocation-different twins in
 * emi/scenario/scena05/00 at 0x801F92EC, 0x801F8C1C, 0x801FC2F0 and 0x801FC778,
 * which share 33 of the 36 words and differ only in the action id handed to
 * func_80150224 (0, 0x26, 0x84 and 0x4C) and in the absolute encodings of the
 * two `j` words, so none of them is clone-eligible; they are shape evidence, and
 * 0x801F92EC is the measured-exact source-shape sibling
 * startActionZeroLatchPhaseByteTwoThenReleaseFrontEndAndRequestPrimaryState0ScenarioScena0500_801F92EC,
 * which supplies the switch-on-sub-state form, the saved sub-state pointer, the
 * delay-slot constants 1 and 2 and the phase-byte gate this body repeats.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startAction34LatchPhaseByteTwoThenReleaseFrontEndAndClearPrimaryStateScenarioScena0100_801FCE30(void) {
  u8 *substate = &D_80146875;

  switch (*substate) {
  case 0:
    func_80150224(0x34);
    D_80143BB0 = 2;
    *substate = 1;
    break;
  case 1:
    if (D_80143BB0 != 2) {
      func_8015C058();
      *substate = 0;
      D_80146874 = 0;
    }
    break;
  }
}
