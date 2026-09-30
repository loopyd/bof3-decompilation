#include "bof3/bof3.h"

extern u16 D_801FE9EC[];

/* @source 0x801F81F4
 * @behavior Overlay unsigned-halfword table-head insert: it shifts the 0x40
 * halfword entries of the table at 0x801FE9EC one slot toward higher addresses
 * (entry i takes the old entry i-1, for i running 0x40 down to 1, so the copy is
 * a descending bottom-tested loop with no memory overlap hazard) and then stores
 * its unsigned halfword argument in entry 0, which makes entry 0 the newest
 * value. It reads and writes only that table range (0x801FE9EC .. 0x801FEA6C),
 * takes one u16 argument and returns nothing: the function is a leaf with no
 * stack frame and no saved registers, no call and a plain jr $ra (nop in the
 * delay slot), and it leaves no result in $v0. Shape verified from the bytes: the
 * loop counter is an unsigned byte, so both the element index and the bottom loop
 * test are zero-extended with andi 0xFF; the source base is hoisted once before
 * the loop as the destination base minus 2 bytes (addiu), so the entry i-1 of the
 * source folds into that base rather than into a memory offset; one element index
 * is scaled once (sll by 1) and added to both bases; the argument is stored
 * straight from $a0 with no masking (sh $a0, 0(%lo(D_801FE9EC))). Its three
 * callers each fill the table newest-first: func_801F70C0 (jal at 0x801F710C),
 * func_801F743C (jal at 0x801F7488) and func_801F8534 (jal at 0x801F85B4) run a
 * 0x40-iteration loop that adds (rand() & 0xFF) to the unsigned halfword at +0x58
 * of the scratchpad work object reached through the pointer slot 0x1F800044 and
 * pass the running total here. The table is read back only by func_801F823C,
 * which loads (&D_801FE9EC)[byte & 0xFF] (at 0x801F82E4) and hands the entry to
 * the shared rotation helper func_801782FC that the exact lift func_801F2E3C
 * also applies; both this address and the table have no other reference in the
 * image.
 * @status exact
 * @match 100.00
 * @residual none
 */
void shiftTableEntriesAndInsertHeadValueScenarioScena0800_801F81F4(u16 arg0) {
  u8 i;

  for (i = 0x40; i != 0; i--) {
    D_801FE9EC[i] = D_801FE9EC[i - 1];
  }
  D_801FE9EC[0] = arg0;
}
