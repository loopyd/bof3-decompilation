#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC5E4
 * @behavior Overlay per-record callback: it runs the shared front-end startup
 * helper func_8015C088 (0x8015C088), clears the three shared main-RAM state
 * bytes 0x80146865, 0x80146866 and 0x80146867, clears the shared sub-state byte
 * D_80146875 and then requests primary scene state 1 by storing 1 in the shared
 * primary state byte D_80146874. It takes no arguments and returns nothing; the
 * 0x18-byte frame only keeps $ra across the call. Each of the five byte stores
 * materialises the 0x8014 page in $at on its own, and the constant 1 is
 * materialised once in $v0 (the word 0x24020001 immediately after the call) to
 * feed the last store, so the store order 0x80146865, 0x80146866, 0x80146867,
 * D_80146875, D_80146874 is the source statement order. The dispatcher
 * callRecordCallback (0x801FC574)
 * invokes it indirectly through word 6 of the 15-entry overlay callback run at
 * 0x801FD1EC (0x801FD204 holds 0x801FC5E4), so the record byte at offset 0x7A
 * selects it with the value 6; the record pointer and second argument that
 * dispatcher passes are ignored, and no in-image jal reaches the address. The
 * only in-image reference to this address is that table word. Its 76 bytes are
 * the same helper-and-clear shape as its callback-run neighbour
 * stateClearRequest8Sub
 * (0x801FC738), whose own live gate measured exact; comparing those two payload
 * spans, the only differing word is the immediate of that one $v0 load (1 here,
 * 8 there). The name records that proven behavior and the address anchor keeps
 * it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSharedBytesPrimary(void) {
  func_8015C088();
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 1;
}
