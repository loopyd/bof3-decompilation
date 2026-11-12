#include "bof3/bof3.h"

extern u16 D_80143F00;
extern u8 D_80143F03;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C088(void);
void func_8015507C(s32 arg0, s32 arg1);

/* @source 0x801FBA34
 * @behavior Overlay event handler reached only from a table word: no in-image
 * jal names the address, its single image word is the word at 0x801FBF40
 * (payload offset 0x5340), entry 3 of the null-terminated four-entry
 * code-pointer run at 0x801FBF34 whose base is published in emi/etc/game/00 at
 * 0x801C8488. It takes the two packed record words the frame was entered with,
 * saves them in $s0 and $s1 before any call and never writes either, and its
 * 0x20-byte frame keeps exactly those two and $ra.
 *
 * The first arm requires the shared route halfword D_80143F00 to read 0x8F,
 * the shared byte D_80143F03 to read 0, scenario flag bit 0x1C of the shared
 * flag word D_8014686C to be set (func_8015B5D4 nonzero) while bit 0x1D is
 * clear, the first record word's high half to equal 0x2E and the second word's
 * high half to fall in the inclusive range 0x1F..0x22 (the `addiu -0x1F` /
 * `sltiu 4` pair). On that path it runs the shared front-end startup helper
 * func_8015C088, requests primary scene state 8 with sub-state 0 by storing 8
 * in the shared state byte D_80146874 and then 0 in the companion sub-state
 * byte D_80146875, and answers 1 through a `j` to the shared epilogue.
 *
 * The second arm requires the route halfword to read 0x91, D_80143F03 to read
 * 5, flag bit 0xA of D_8014686C to be clear, the first record word's high half
 * to fall in 0x40..0x41 and the second word's high half to equal 0x6A. It sets
 * that flag bit through func_8015B580(D_8014686C, 0xA) and cues
 * func_8015507C(0x40, 0x6B), then answers 0. Every return-0 test materialises
 * its zero as `addu $v0,$zero,$zero` in the branch delay slot; the 0x40..0x41
 * test shares its return site with the trailing one, which is why it branches
 * to the `addu` block that falls into the epilogue instead of past it.
 * @status exact
 * @match 100.00
 * @residual none
 */

/* Name evidence: 0x80143F00 is the shared route halfword the scenario family
 * gates on, 0x80143F03 the companion byte of the same tuple, and
 * 0x80146874/0x80146875 the shared front-selector primary state and sub-state
 * bytes - written 8 then 0 here, the "request primary state N with sub-state M"
 * request the family names spell out. func_8015B5D4 is the shared flag-bit test
 * and func_8015B580 the shared flag-bit set helper of the main executable, both
 * taking the flag word and the bit index; func_8015C088 is the shared front-end
 * startup helper. The two arguments carry a record id in their high half only,
 * which is why every test shifts by 16 first and the low halves stay unread.
 *
 * Shape: statement order, operand forms, guard idiom and declaration style come
 * from two siblings of this target. The disassembly of func_801FB464
 * (0x801FB464), the previous entry of the same four-entry table, is original
 * bytes and shows the same frame, the same route/byte/flag gates and, in its
 * final arm, the same `sra 16` range test followed by func_8015C088 and the
 * D_80146874/D_80146875 store pair with `return 1`; the exact lift
 * requestPrimaryState8Substate2AndArmCountdown30OnPackedRecordMatchScenarioScena0300_801FCD1C
 * supplies the `extern u32 D_8014686C` cell, the `s32 func_8015B5D4(u32, s32)`
 * prototype and the `(u32)((arg >> 16) - c) >= n` early-return guard form.
 * Every literal, bit index, packed range and callee constant is written from
 * this span's own bytes.
 *
 * Clone declined: an exact scan of the 0x144-byte body over every extracted
 * payload under out/binaries matched only this span itself, at offset 0x4E34 of
 * emi/scenario/scena13/00.bin. Data classification declined: the span opens
 * with `addiu $sp,$sp,-0x20`, closes with `jr $ra` and holds ten calls, so it is
 * a function rather than a table or a fragment head.
 */
s32 requestPrimaryState8Substate0OnRoute8FPackedRangesOrSetFlagBit10OnRoute91PackedRangeScenarioScena1300_801FBA34(
    s32 arg0, s32 arg1) {
  if (D_80143F00 == 0x8F && D_80143F03 == 0 &&
      func_8015B5D4(D_8014686C, 0x1C) != 0 &&
      func_8015B5D4(D_8014686C, 0x1D) == 0 && (arg0 >> 16) == 0x2E &&
      (u32)((arg1 >> 16) - 0x1F) < 4u) {
    func_8015C088();
    D_80146874 = 8;
    D_80146875 = 0;
    return 1;
  }

  if (D_80143F00 != 0x91) {
    return 0;
  }

  if (D_80143F03 != 5) {
    return 0;
  }

  if (func_8015B5D4(D_8014686C, 0xA) != 0) {
    return 0;
  }

  if ((u32)((arg0 >> 16) - 0x40) >= 2u) {
    return 0;
  }

  if ((arg1 >> 16) != 0x6A) {
    return 0;
  }

  func_8015B580(D_8014686C, 0xA);
  func_8015507C(0x40, 0x6B);
  return 0;
}
