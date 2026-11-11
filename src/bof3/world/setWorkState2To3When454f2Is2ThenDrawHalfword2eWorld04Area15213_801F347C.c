#include "bof3/bof3.h"

/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;

/* The local drawer narrows both arguments with `sll`/`sra 16` in its own
 * prologue and neither call site consumes a result. */
void func_801F39C4(s16 arg0, s16 arg1);

/* @source 0x801F347C
 * @behavior Byte-2 step of this overlay's handler run. When the shared mode
 *           byte at 0x801454F2 reads 2 it writes 3 to byte 0x02 of the work
 *           record published at the scratchpad pointer slot 0x44; it then
 *           reloads that slot and calls the local drawer 0x801F39C4 with 0x10
 *           and the signed halfword at record offset 0x2E, discarding any
 *           result. Its only in-image `jal` caller is 0x801F3420, which first
 *           adds 0x10 to that same halfword and bumps work byte 0x02 once the
 *           advanced signed value reaches 0x10; the word 0x801F347C also sits
 *           at 0x801F4D7C, entry 9 of the in-image code-pointer run running
 *           from 0x801F4D58 through 0x801F4D90. It takes no arguments, uses no
 *           result, saves only the return address at 0x10 of its 0x18-byte
 *           frame and touches no other state. The 0x50 bytes match the reviewed
 *           setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld03Area12114_801F35D0
 *           in emi/world03/area121/14 apart from the single internal `jal` word
 *           at offset 0x38, which carries each overlay's own drawer address
 *           (0x801F39C4 here, 0x801F3B18 there), so this body takes that
 *           sibling's statement order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld04Area15213_801F347C(
    void) {
  u8* record;

  if (D_801454F2 == 2) {
    SPAD_PTR_SLOT(u8, 0x44)[2] = 3;
  }
  record = SPAD_PTR_SLOT(u8, 0x44);
  func_801F39C4(0x10, FIELD_REF(s16, record, 0x2E));
}
