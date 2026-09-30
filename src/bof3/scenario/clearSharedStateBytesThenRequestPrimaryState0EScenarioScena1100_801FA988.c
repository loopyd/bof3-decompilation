#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FA988
 * @behavior Overlay per-record callback: it runs the shared front-end startup
 * helper func_8015C088 (0x8015C088), clears the three shared main-RAM state
 * bytes 0x80146865, 0x80146866 and 0x80146867, clears the shared secondary
 * sub-state byte D_80146875 and then requests primary scene state 0x0E by
 * storing 0x0E in the shared primary state byte D_80146874. It takes no
 * arguments, returns nothing and reads no memory of its own; the 0x18-byte
 * frame only keeps $ra across the call. Each of the five byte stores
 * materialises the 0x8014 page in $at on its own, the constant 0x0E is
 * materialised once in $v0 (`addiu $v0,$zero,0xE` immediately after the call)
 * and the four zero stores use $zero, so the store order 0x80146865,
 * 0x80146866, 0x80146867, D_80146875, D_80146874 is the source statement
 * order. The dispatcher dispatchRecordCallbackScenarioScena1100_801FA5B8
 * invokes it indirectly through word 12 of the 29-entry overlay callback run at
 * 0x801FAF24 (0x801FAF54 holds 0x801FA988), so the record byte at offset 0x7A
 * selects it with the value 0x0C; the record pointer and second argument that
 * dispatcher passes are ignored, and neither a jal nor a j anywhere in the
 * image reaches the address. Its 76 bytes are the same helper-and-clear shape
 * as the exact neighbours
 * clearSharedStateBytesThenRequestPrimaryState3ScenarioScena1100_801FA674
 * (0x801FA674, stores 3),
 * clearSharedStateBytesThenRequestPrimaryState5ScenarioScena1100_801FA6C0
 * (0x801FA6C0, stores 5),
 * clearSharedStateBytesThenRequestPrimaryState6ScenarioScena1100_801FA75C
 * (0x801FA75C, stores 6),
 * clearSharedStateBytesThenRequestPrimaryState7ScenarioScena1100_801FA7A8
 * (0x801FA7A8, stores 7) and
 * clearSharedStateBytesThenRequestPrimaryState9ScenarioScena1100_801FA7F4
 * (0x801FA7F4, stores 9): all of them run the same helper, clear the same four
 * bytes in the same order and differ only in the one stored constant; the
 * address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedStateBytesThenRequestPrimaryState0EScenarioScena1100_801FA988(void) {
  func_8015C088();
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 0xE;
}
