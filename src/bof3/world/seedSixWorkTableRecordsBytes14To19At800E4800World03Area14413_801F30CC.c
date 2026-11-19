#include "bof3/bof3.h"

/* @source 0x801F30CC
 * @behavior Overlay step-0 bootstrap of the shared work table: seeds bytes 0x14
 *           through 0x19 of each of the six 0x1C-byte-stride records of the
 *           main-RAM work table at 0x800E4800 (the work area this overlay shares
 *           with the world and scenario overlays), walking the block with an
 *           unsigned 16-bit counter in a bottom-tested loop that adds 0x1C to
 *           the record pointer each pass. The six seeded bytes are, in ascending
 *           record offset, 0x00, 0xF0, 0x00, 0x03, 0x01, 0x1E; the stores are
 *           issued in the order 0x18, 0x19, 0x17, 0x14, 0x15, 0x16, which is the
 *           order the original issues them. It takes no arguments, returns
 *           nothing, calls nothing and reads no state. The body reserves 0x10
 *           bytes of stack and uses none of them: there is no spill, no saved
 *           $ra and no call, so it is a leaf. Its only in-payload caller is
 *           func_801F2CA4 (`jal` at 0x801F2CC0), which runs it once when
 *           func_801F34FC answers zero, then requests resource 0x203 and
 *           advances byte +0x01 of the work record published through the
 *           scratchpad pointer cell 0x1F800044; that caller loads its own
 *           constants and ignores $v0, which agrees with the no-argument void
 *           prototype. The `addiu $sp` pair and the `jr $ra` at 0x801F311C prove
 *           the 0x58-byte boundary up to the next entry at 0x801F3124.
 *
 *           Representation notes: the loop pointer is anchored at the table base
 *           0x800E4800 and the six stores carry the record offsets 0x14..0x19, so
 *           GCC 2.6.3 strength-reduces the pointer and combines its five address
 *           givs onto the +0x16 one, materialising the single base register
 *           0x800E4816 with the displacements -0x2..0x3 the original shows. Seeds
 *           anchored at 0x800E4814 or 0x800E4816 with byte offsets -0x2..0x3 kept
 *           a second address register and one extra increment (25 instructions,
 *           9-11 of 22 matching), because a store at the biv's own value leaves
 *           that biv live for the giv combination; anchoring at the table base
 *           leaves no such access and the biv is eliminated.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedSixWorkTableRecordsBytes14To19At800E4800World03Area14413_801F30CC(void) {
  /* Reserved frame only; no access and no use. Reconstruction choice. */
  u8 scratch[0x10];
  u8* work;
  u16 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    work[0x18] = 1;
    work[0x19] = 0x1E;
    work[0x17] = 3;
    work[0x14] = 0;
    work[0x15] = 0xF0;
    work[0x16] = 0;
    work += 0x1Cu;
    i += 1u;
  } while (i < 6u);
}
