#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;

s32 func_8015C088(void);

/* @source 0x801FC6C4
 * @behavior The shared helper func_8015C088 is declared value-returning in this
 * translation unit: the shipped bytes materialise the two stored constants in
 * $v1, i.e. $v0 is referenced across the three stores after the call, which is
 * what a value-returning declaration produces here.
 * Slot 8, the word 0x801FC6C4 at 0x801FD02C that closes the overlay's nine
 * code-pointer words at 0x801FD00C..0x801FD02C (D_801FD00C), which
 * dispatchRecordCallbackScenarioScena0300_801FC46C selects with the record's
 * unsigned byte at offset 0x7A and calls as (record, D_8014686C): it runs the
 * shared front-end startup helper func_8015C088 and then requests primary scene
 * state 7 by storing 7 in the shared state byte D_80146874 while arming the
 * shared unsigned halfword countdown D_80146876 with 0x1E (30) and leaving the
 * shared sub-state byte D_80146875 at 0 (0x1E, then 7, then 0, in that order),
 * and finally returns 0 to the caller. The record pointer in $a0 and the shared
 * word in $a1 are never read; the 0x18-byte frame only keeps $ra across the
 * call, every store materialises the 0x8014 page in $at on its own, which is how
 * the psyq compiler emits this overlay's separate shared-object stores, and the
 * third store uses $zero directly, so the shipped body stores literal
 * constants. D_80146876 is the shared countdown word the same scenario family
 * loads, decrements and re-arms (0x1E, 0x20, 0x130, 0x200).
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801FC6C4(void) {
  func_8015C088();
  D_80146876 = 0x1E;
  D_80146874 = 7;
  D_80146875 = 0;
  return 0;
}
