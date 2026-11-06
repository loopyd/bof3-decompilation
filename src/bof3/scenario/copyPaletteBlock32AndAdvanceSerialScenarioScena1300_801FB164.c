#include "bof3/bof3.h"

/* @source 0x80145988 @kind unknown */
extern volatile u8 paletteStageSerial;

/* @source 0x801FB164
 * @behavior Copies the sixteen halfwords (32 bytes) of the palette source
 * staging block at 0x80033800 into the VRAM shadow block at 0x80037800 and then
 * advances the shared palette-stage serial byte at 0x80145988 by one; takes no
 * arguments and returns nothing. Evidence for the shapes: the two block bases
 * are built as plain integer constants (`lui`/`ori` pairs, no `%hi`/`%lo`); the
 * copy is the indexed form the byte-identical exact sibling copyPaletteBlock
 * (0x801F845C in emi/scenario/scena16/00) uses, which strength-reduces to the
 * original's bottom-tested loop with the `slti 0x10` counter and the
 * destination increment in the branch delay slot; the serial read folds into
 * its own `lui`/`lbu` pair and the store re-materialises `$at` for a second
 * `lui`, the bare-symbol form.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit is instruction- and byte-exact: 20/20 instructions, 80 bytes. The
 * shape sweep measured the indexed sibling form, the pointer-increment form, the
 * inline serial read-modify-write and a volatile byte-base serial access all at
 * 20/20 and 80 bytes; the retained form is the indexed one with the named serial
 * symbol. No bytes outside this function change.
 */
void copyPaletteBlock32AndAdvanceSerialScenarioScena1300_801FB164(void) {
  volatile u16 *dst;
  const volatile u16 *src;
  s32 i;
  u8 serial;

  i = 0;
  dst = PSX_PTR(volatile u16, 0x80037800u);
  src = PSX_PTR(const volatile u16, 0x80033800u);

  do {
    dst[i] = src[i];
    i++;
  } while (i < 0x10);

  serial = paletteStageSerial;
  serial = serial + 1;
  paletteStageSerial = serial;
}
