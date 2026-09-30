#include "bof3/bof3.h"

/* Shared overlay helper invoked once the work object's byte at offset 0x0A
 * reaches 8; the same extern address is lifted by invokeBmagicHelperMagic004.c
 * (emi/bmagic/magic004/03@0x801EFAD4) and by the area000/area003 overlays. */
void func_801E5988(void);

/* @source 0x801F02B4
 * @behavior Advances the scratchpad work object's byte at offset 0x0A by one
 * and, when the resulting byte equals 8, invokes the shared helper at
 * 0x801E5988 with no arguments. The pointer cell at 0x1F800044 is read once,
 * the byte is loaded with lbu, incremented with addiu 1, stored back with sb
 * and then zero-extended with andi 0xFF for the byte-width comparison against
 * the literal 8, which is materialized as addiu $v0,$zero,8. The pointer cell
 * is loaded before the 0x18-byte frame is created and that frame only keeps
 * $ra across the helper call. Nothing is returned and no other state is read
 * or written. This address is also the sixth word (index 5) of the in-image
 * code pointer run based at 0x801F08C4, whose words are 0x801F00A0, 0x801F00EC,
 * 0x801F014C, 0x801F01D0, 0x801F0230 and 0x801F02B4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAThenInvokeHelperAt8BmagicMagic07803_801F02B4(void) {
  u8* work = SPAD_PTR_SLOT(u8, 0x44u);
  u8 next = work[0xA] + 1;

  work[0xA] = next;
  if (next == 8) {
    func_801E5988();
  }
}
