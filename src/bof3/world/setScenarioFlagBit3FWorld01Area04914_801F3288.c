#include "bof3/bof3.h"

/* Shared scenario flag word of the main executable. */
extern u32 D_8014686C;

/* @source 0x8015B580 - shared block bit-set helper (owner, bit). */
extern void func_8015B580(u32 arg0, u8 bit_index);

/* @source 0x801F3288
 * @behavior Reads the shared scenario flag word D_8014686C and hands it to the
 *           shared block bit-set helper at 0x8015B580 with bit 0x3F, then
 *           returns. Takes no arguments and returns nothing; its 0x18-byte
 *           frame only keeps $ra across the call. Neither an in-image jal nor
 *           an image pointer word names this address, so the caller stays the
 *           overlay dispatch of the main executable.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit3FWorld01Area04914_801F3288(void) {
  func_8015B580(D_8014686C, 0x3F);
}
