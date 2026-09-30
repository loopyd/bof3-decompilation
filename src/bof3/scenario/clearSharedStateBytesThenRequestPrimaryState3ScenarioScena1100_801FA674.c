#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FA674
 * @behavior Overlay per-record callback: it runs the shared front-end startup
 * helper func_8015C088 (0x8015C088), clears the three shared main-RAM state
 * bytes 0x80146865, 0x80146866 and 0x80146867, clears the shared secondary
 * sub-state byte D_80146875 and then requests primary scene state 3 by storing
 * 3 in the shared primary state byte D_80146874. It takes no arguments, returns
 * nothing, and reads no memory of its own; the 0x18-byte frame only keeps $ra
 * across the call. Each of the five byte stores materialises the 0x8014 page in
 * $at on its own, and the constant 3 is materialised once in $v0 (`addiu
 * $v0,$zero,0x3` immediately after the call) to feed the last `sb`, so the
 * store order 0x80146865, 0x80146866, 0x80146867, D_80146875, D_80146874 is the
 * source statement order. The dispatcher
 * dispatchRecordCallbackScenarioScena1100_801FA5B8 invokes it indirectly
 * through word 2 of the 29-entry overlay callback run at 0x801FAF24 (0x801FAF2C
 * holds 0x801FA674), so the record byte at offset 0x7A selects it with the value
 * 0x02; the record pointer and second argument that dispatcher passes are
 * ignored, and no in-image jal reaches the address. Its 76 bytes are the same
 * helper-and-clear shape as the still-unlifted immediate neighbour func_801FA6C0
 * (0x801FA6C0), which likewise runs func_8015C088, clears the same three bytes
 * and D_80146875 and requests primary state 5, differing in the one stored
 * constant; the address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedStateBytesThenRequestPrimaryState3ScenarioScena1100_801FA674(void) {
  func_8015C088();
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 3;
}
