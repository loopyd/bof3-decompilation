#include "bof3/bof3.h"

/* Shared 164-byte field-record array the handler reads its code byte from. */
extern u8 D_80144982[]; /* @source 0x80144982 @kind data */

/* Shared main-RAM counter the bank selector tests against 5. */
extern u8 D_80144F56; /* @source 0x80144F56 @kind unknown */

/* This payload's own three-word record bank table, the last twelve bytes of
 * the payload: 0x801F38D0, 0x801F3938 and 0x801F39A0, the bases of the three
 * six-record runs of 0x11-byte records that follow it. */
extern u8 *D_801F3A08[]; /* @source 0x801F3A08 @kind table */

s32 func_8015B5D4(u32 arg0, s32 arg1);
s32 func_8015CA48(void);
void func_801A4D54(void *arg0);

/* @source 0x801F3444
 * @behavior Activates up to six records of one of this payload's three record
 * banks. It first reads the code byte at 0x80144982 and asks the shared wait
 * helper 0x8015B5D4 about the main-RAM bank D_80144F28 with flag id 0x77: a
 * nonzero answer retires the handler before anything is activated. Otherwise
 * it asks the same helper about the second main-RAM bank D_80144EF8 with id
 * 0x0A and folds that answer together with the code byte and the shared
 * counter D_80144F56 into a bank selector: a pending id 0x0A wait takes bank
 * 2 unless the code byte stays below 5, and a zero answer takes bank 2 for a
 * code byte of 9 or more, bank 1 for 5 through 8, and for a code byte below 5
 * either bank 1 when the counter reads 5 or more or bank 0 when it reads less.
 * It then walks the six 0x11-byte records of the selected bank in order,
 * stores the halfword answer of the random helper 0x8015CA48 into the
 * scratchpad work halfword 0x1F800000, and activates each record whose answer
 * is not 0xFF through 0x801A4D54. The 0x20-byte frame keeps $ra, $s2, $s1 and
 * $s0; the interior base 0x80144982 is materialized once and both wait banks
 * are addressed as that base plus 0x5A6 and plus 0x576, and the two code-byte
 * tests read the same zero-extended value twice in the id 0x0A-zero arm, which
 * is the `andi` this build shares there while the one-use arm in the pending
 * arm folds it into its `sltiu`.
 *
 * Name evidence: the flag 0x77 tested on the D_80144F28 bank is the
 * `addiu $a1, $zero, 0x77` argument of the first 0x8015B5D4 call, the id 0x0A
 * wait on 0x80144EF8 is the `addiu $a1, $zero, 0xA` argument of the second,
 * the three candidate banks are the three pointer words of 0x801F3A08 scaled
 * by four, the six-record walk is the `sltiu 0x6` loop bound with the
 * `sll 4`/`addu` product that scales the index by 0x11, the randomized
 * activation gate is the `beq` against the 0xFF constant in $s2 after the
 * `sll 16`/`sra 16` of the 0x8015CA48 answer, and the activation call is the
 * 0x801A4D54 `jal`. No extracted payload holds this address as a call target
 * or table word, so the handler is reached from outside the payload and its
 * role stays address-anchored; the same body sits at emi/world04/area191/13
 * 0x801F39AC, whose two ranges differ in three words only, the two internal
 * `j` targets that shift by the span delta 0x568 and the low half of the
 * 0x801F43D8 bank table immediate that does not, so that sibling was shape
 * evidence with its own table constant rather than a clone candidate, and it
 * was still an `asm` row in its own target.
 *
 * Shape: seeded from that sibling's statement order and written with this
 * span's own table base, both wait bank offsets and both call ids from the
 * original bytes. Measured levers: a masked scan over every extracted payload
 * found only this range, so clone is ineligible; the range has its own
 * prologue and `jr $ra` epilogue at the payload's own boundary, so the data
 * lever is ineligible; and it starts at its own declared entry and ends at the
 * next declared one, so carve has no fragment to regroup.
 * @status exact
 * @match 100.00
 * @residual none
 */
void activateSixRecordsOfCodeAndWaitSelectedBankUnlessFlag77OrRandomFFWorld04Area19213_801F3444(
    void) {
  u8 *base;
  u8 code;
  s16 value;
  u32 bank;
  u8 i;

  base = D_80144982;
  code = base[0];
  if (func_8015B5D4((u32)(base + 0x5A6), 0x77) != 0) {
    return;
  }
  if (func_8015B5D4((u32)(base + 0x576), 0x0A) == 0) {
    if (code < 9) {
      if (code < 5) {
        bank = D_80144F56 >= 5;
      } else {
        bank = 1;
      }
    } else {
      bank = 2;
    }
  } else {
    if (code < 5) {
      bank = D_80144F56 >= 5;
    } else {
      bank = 2;
    }
  }
  for (i = 0; i < 6; i++) {
    value = func_8015CA48();
    SPAD_REF(u16, 0x00) = value;
    if (value != 0xFF) {
      func_801A4D54(D_801F3A08[bank] + i * 0x11);
    }
  }
}
