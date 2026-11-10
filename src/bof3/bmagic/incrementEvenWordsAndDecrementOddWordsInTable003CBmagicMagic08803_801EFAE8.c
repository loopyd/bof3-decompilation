#include "bof3/bof3.h"

/* Six-word state table carried inside this overlay, at the head of the
 * reviewed textbin run that starts at 0x801EFFE0. This body is its only
 * consumer in the payload: the base is materialised once before the loop and
 * the pointer advances by four per iteration, so the table is read and written
 * in place at word width.
 * @source 0x801F003C @kind table
 */
extern u32 D_801F003C[];

/* @source 0x801EFAE8
 * @behavior Per-frame step over the six words of this payload table at
 *           0x801F003C (words 0x801F003C through 0x801F0050). One pointer
 *           walks the table by four bytes, the loop index is masked with one
 *           so odd indices take the decrement arm, and the single computed
 *           word is stored back to the cell it came from: every even-indexed
 *           word gains one and every odd-indexed word loses one. The base
 *           address is materialised once before the loop (lui plus addiu) and
 *           the index bound is the immediate six (slti), so the body is a leaf
 *           with no frame that takes no arguments, returns nothing and reads
 *           or writes no other state. The index is zeroed before the table
 *           base is materialised, which is the order the original emits those
 *           two preheader instructions in. No jal in this payload reaches it
 *           and no pointer word anywhere in the extracted corpus names it, so
 *           its caller is the overlay loader or a script-invoked callback and
 *           its argument roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementEvenWordsAndDecrementOddWordsInTable003CBmagicMagic08803_801EFAE8(void) {
  s32 index;
  u32 value;
  u32 *words;

  index = 0;
  words = D_801F003C;
  for (; index < 6; index++) {
    if (index & 1) {
      value = *words - 1;
    } else {
      value = *words + 1;
    }
    *words = value;
    words++;
  }
}
