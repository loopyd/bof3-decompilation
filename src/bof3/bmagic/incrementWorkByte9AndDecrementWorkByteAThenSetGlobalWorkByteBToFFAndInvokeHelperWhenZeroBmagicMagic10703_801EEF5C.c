#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. The cell is re-evaluated for
 * every access group because a byte store through the loaded pointer may alias
 * it, so the compiler re-materializes `lui $vN,0x1F80` / `lw $vN,0x44($vN)`
 * exactly where the original does (three times in this body). Cell spelling and
 * statement order follow this target's exact siblings
 * incrementWorkByte9AndAdvanceWorkByte1AtC0BmagicMagic10703_801EEF0C
 * (0x801EEF0C) and clearWorkBytes9ABAndAdvanceWorkByte1BmagicMagic10703_801EEE50
 * (0x801EEE50), which use the same macro cell. */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * Splat binds 0x801EB4E0 through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. Byte 0xB of that
 * record is the work step the exact siblings of the battle overlays read and
 * rewrite. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared battle helper in main RAM. The generated undefined_funcs_auto.txt row
 * covers the address, and the still-asm row func_801EED40 of this target calls
 * it too, so no target-map row is needed. */
void func_801E5988(void);

/* @source 0x801EEF5C
 * @behavior Per-frame handler of the battle work object published at the
 *           scratchpad pointer cell 0x1F800044. It increments that object's
 *           unsigned byte 0x09 by one at byte width (lbu / addiu 1 / sb),
 *           re-reads the cell and subtracts one from unsigned byte 0x0A the
 *           same way, then re-reads the cell a third time and tests the byte
 *           0x0A it finds there, so the compared value is a fresh load and no
 *           truncation of a stored register supplies it. Only when that byte is
 *           zero does it read the shared battle work-record pointer at
 *           0x801EB4E0, store 0xFF into that record's byte 0x0B and invoke the
 *           shared helper at 0x801E5988: the 0xFF materialization fills the
 *           delay slot of the bnez while the store fills the delay slot of the
 *           call. The first cell load is issued before frame creation because
 *           the increment needs no stack; the 0x18-byte frame only keeps $ra at
 *           0x10 across the call. Takes no arguments, returns nothing and reads
 *           or writes no other state. No jal and no in-image word in this
 *           payload names this 120-byte span, and no byte-identical copy of it
 *           exists in any bmagic overlay payload of this repository, so the
 *           entry is reached from outside the archive. Statement order, operand
 *           forms and declaration style follow the exact siblings
 *           incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteB
 *           AndInvokeHelperWhenZeroBmagicMagic14503_801EF0A0 (same byte 0x09 and
 *           0x0A read-modify-writes, same work-record byte 0x0B arm and the same
 *           helper call) and
 *           incrementWorkByte9ThenSetGlobalWorkByteBToFFAndInvokeHelperWhenWork
 *           Byte9Is20BmagicMagic09403_801EF0FC (same 0xFF store into that record
 *           byte, materialized in the branch delay slot).
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndDecrementWorkByteAThenSetGlobalWorkByteBToFFAndInvokeHelperWhenZeroBmagicMagic10703_801EEF5C(void)
{
  u8** slots;

  slots = SPAD_PTR_TABLE(u8);

  slots[0x11][0x09] += 1;
  slots[0x11][0x0A] -= 1;
  if (slots[0x11][0x0A] == 0) {
    D_801EB4E0[0x0B] = 0xFF;
    func_801E5988();
  }
}
