#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015C088(void);

/* @source 0x801FC690
 * @behavior Slot 7 of the overlay's nine-pointer handler table at 0x801FD00C
 * (D_801FD00C), which the record dispatcher func_801FC46C selects with byte
 * 0x7A of its record argument and calls as (record, D_8014686C): it runs the
 * shared front-end startup helper func_8015C088 and then requests primary scene
 * state 6 with a cleared secondary sub-state by storing 6 in the shared state
 * byte D_80146874 and 0 in the shared sub-state byte D_80146875, and finally
 * returns 0 for the caller. The record pointer in $a0 and the shared scenario
 * flag word in $a1 are never read; the 0x18-byte frame only keeps $ra across
 * the call, and each of the two byte stores materialises the 0x8014 page in
 * $at on its own, which is how the psyq compiler emits separate shared-byte
 * stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801FC690(s32 arg0, s32 arg1) {
  func_8015C088();
  D_80146874 = 6;
  D_80146875 = 0;
  return 0;
}
