#include "bof3/bof3.h"

/* Record array of this overlay's battle work area: 0xC0 records of 0x1C bytes
 * starting at 0x801F13B4, inside the payload's trailing data run that splat
 * declares at 0x801F138C. The target's generated undefined-symbol table binds
 * this base through the word at 0x801F13B4. Record byte 0 is the claim byte:
 * the local helper at 0x801F03D8 stores 1 into it for up to six records whose
 * byte 0 is still zero, and the record dispatcher at 0x801F0448 selects a step
 * with record byte 1. The 0x1C-byte record width is carried by the array type,
 * which is what makes the original recompute the base per step as a folded
 * lui $at / addu $at address instead of hoisting the array address into a
 * general register: the same body with a flat byte array declared and the scale
 * spelled i * 0x1C measured 24 of the 29 original instructions and grew the
 * span 116 to 120 bytes, first difference at +0x000c. */
extern u8 D_801F13B4[][0x1C]; /* @source 0x801F13B4 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-symbol table binds 0x801EB4E0, so the name
 * needs no target-map row, and the record is read only through the byte view
 * below. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared helper of the main executable at 0x801E5988, outside this overlay's
 * image; the payload only calls it. */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801F0364
 * @behavior Walks the overlay work records at 0x801F13B4 in order, keeping the
 *           unsigned 16-bit index in a masked copy per step, and returns as
 *           soon as a record byte 0 is non-zero, which marks a record the local
 *           helper at 0x801F03D8 has already claimed by storing 1 there. When
 *           all 0xC0 records the unsigned bound allows are still unclaimed, it
 *           reads the shared battle work record pointer at 0x801EB4E0 once,
 *           clears bit 0 of that record byte at offset 0xB at byte width
 *           (lbu 0xB, andi 0xFE, sb 0xB), and calls the shared helper at
 *           0x801E5988, that store filling the call delay slot. The payload
 *           pointer word at 0x801EEC6C, offset 0x6C of the overlay pointer run
 *           based at 0x801EEC04, is its only in-image reference, so no
 *           in-payload call names it: it consumes no arguments and its result
 *           is unused. The 0x18-byte frame only keeps $ra at 0x10. The
 *           0x74-byte span has no byte-identical counterpart in any other emi
 *           payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByteBBit0ThenInvokeHelperWhenNoRecordByte0SetBmagicMagic06403_801F0364(
    void) {
  u16 i;

  for (i = 0; i < 0xC0; i++) {
    if (D_801F13B4[i][0] != 0) {
      return;
    }
  }
  D_801EB4E0[0xB] &= 0xFE;
  func_801E5988();
}
