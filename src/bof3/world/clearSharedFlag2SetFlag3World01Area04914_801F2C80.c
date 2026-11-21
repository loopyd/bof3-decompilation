#include "bof3/bof3.h"

/* Shared flag bank of the main executable. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Shared block bit-clear service of the main executable (bank, bit). */
extern void func_8015B5A8(u32 arg0, u8 bit_index);

/* Shared block bit-set service of the main executable (bank, bit). */
extern void func_8015B580(u32 arg0, u8 bit_index);

/* @source 0x801F2C80
 * @behavior Target-local flag transition of this overlay: clears bit 2 and
 *           then sets bit 3 of the shared flag bank at 0x80144F28 through the
 *           shared block bit-clear service 0x8015B5A8 and the shared block
 *           bit-set service 0x8015B580. Takes no arguments, uses its 0x18-byte
 *           frame to keep $s0 and $ra across the two calls, and returns
 *           nothing.
 *
 *           Constant and call evidence from the original bytes: the bank base
 *           is materialised once with `lui`/`addiu` into $s0 and handed to both
 *           calls with `addu $a0,$s0,$zero`; the bit index arrives as
 *           `addiu $a1,$zero,0x2` in the delay slot of the `jal` to 0x8015B5A8
 *           and as `addiu $a1,$zero,0x3` in the delay slot of the `jal` to
 *           0x8015B580, so the clear precedes the set. The epilogue reloads $ra
 *           from 0x14 and $s0 from 0x10, drops the frame and returns through
 *           `jr $ra` with a delay `nop`, so the 0x40-byte span is one complete
 *           leaf with a single return path and no argument register live at
 *           entry.
 *
 *           Shared roles: the same-target span 0x801F2CC0 calls this address
 *           when its argument word is 0x4D8000, and 0x801F3288 shows the same
 *           0x18-byte $ra-only frame around a single call to 0x8015B580 with a
 *           bit index. Which service clears and which sets was fixed by the
 *           reviewed body at 0x801F309C of emi/world00/area026/13, which gated
 *           exact and passed its set index to 0x8015B580 and its clear index
 *           to 0x8015B5A8 against the same kind of shared bank.
 *
 *           Shape: no payload under out/binaries carries this 64-byte span -
 *           one exact-span scan over every payload matches only this target at
 *           offset 0x80 - so the statement order, literals and bindings come
 *           from these bytes rather than from a sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedFlag2SetFlag3World01Area04914_801F2C80(void) {
  func_8015B5A8((u32)&D_80144F28, 0x02);
  func_8015B580((u32)&D_80144F28, 0x03);
}
