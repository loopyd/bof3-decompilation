#include "bof3/bof3.h"

/* Shared main-executable cells of the front-end state block. Both lie outside
 * this overlay's payload; this target's generated undefined-syms map binds
 * them, so no target-map row is needed. */
/* @source 0x80143F03 @kind data */
extern u8 D_80143F03;
/* @source 0x80144F28 @kind data */
extern u8 D_80144F28;

/* Shared flag-bit clear of the main executable. */
/* @source 0x8015B5A8 @kind function */
extern void func_8015B5A8(void* flag_bank, u8 bit_index);

/* @source 0x801F3524
 * @behavior Clears bit 0x74 of the shared main-RAM flag bank at 0x80144F28
 *           through the shared bit-clear helper func_8015B5A8 whenever the
 *           shared front-side byte D_80143F03 reads anything other than 1;
 *           that byte reading 1 touches nothing else. Takes no arguments and
 *           returns nothing; the 0x18-byte frame only keeps $ra across the
 *           call and both paths return through the single shared epilogue.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedFlag74WhenFrontSideByteIsNot1World04Area16913_801F3524(void) {
  if (D_80143F03 != 1) {
    func_8015B5A8(&D_80144F28, 0x74);
  }
}
