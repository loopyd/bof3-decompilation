#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The D_<ADDRESS> spelling auto-binds to 0x801EB4E0 through the generated
 * undefined_syms_auto.txt, so no target-map row is needed. The exact siblings
 * incrementWorkByteBAndDecrementWorkByteAByTwoThenDecrementGlobalWorkByteBAnd
 * InvokeHelperWhenZeroBmagicMagic09803_801EF498 and
 * seedWorkMotionFieldsWhenGlobalWorkByteBIs1BmagicMagic01500_801EFD7C store
 * through and read the same record. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF488
 * @behavior Handler of the work object published at the scratchpad pointer cell
 *           0x1F800044. It increments the object's unsigned byte 0xB by one at
 *           byte width (lbu / addiu 1 / sb), then subtracts two from unsigned
 *           byte 0xA the same way (lbu / addiu -2 / sb), then re-reads byte 0xA
 *           and tests it for zero. Only when the countdown reaches zero does it
 *           read the shared battle work record pointer 0x801EB4E0, decrement
 *           that record's byte 0xB by one and invoke the shared helper at
 *           0x801E5988, whose store fills the call delay slot. Frame 0x18 holds
 *           only $ra, because of the call; the function takes no arguments,
 *           returns nothing and touches no other state.
 *
 *           Each of the three access groups re-evaluates the cell: a byte store
 *           through the loaded pointer may alias 0x1F800044, so the original
 *           re-materialises `lui $vN,0x1F80` / `lw $vN,68($vN)` before every
 *           group. The cell is therefore spelled as index 0x11 of the
 *           scratchpad pointer table, whose zero base keeps the address an
 *           offset load; spelling the cell as the full constant 0x1F800044 makes
 *           the compiler materialise `lui/ori` once and reuse it, which is the
 *           measured failure of the first candidate. The zero test re-reads
 *           byte 0xA rather than truncating the stored value, so a plain u8
 *           accumulator is enough here; the exact sibling
 *           decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAnd
 *           InvokeHelperWhenZeroBmagicMagic15103_801F0CB0 needed the s8 view only
 *           because its original compared the stored register after `sll 24`.
 *
 *           This 136-byte, 34-instruction span is byte-identical (sha256
 *           33115358b3638c2cf00e41ec8a494b999c16eb5f598484ce88ec2af84de8ecb2) to
 *           the already-exact sources of
 *           emi/bmagic/magic098/03 (0x801EF498) and emi/bmagic/magic151/03
 *           (0x801F1380), proven with one payload comparison, and equally
 *           identical to the still-asm row of emi/bmagic/magic143/03
 *           (0x801EF100). The clone adapter declined this range because it
 *           cannot rewrite the representative's cell macro (SPAD_PTR_TABLE is
 *           not a target-map name), so the body copies the sibling shape onto
 *           this target's declaration set directly.
 *
 *           The address is the fifth word (index 4) of the five-entry code
 *           pointer run at 0x801F0AC0 (0x801EEFC8, 0x801EF114, 0x801EF1A4,
 *           0x801EF324, 0x801EF488); no instruction in this payload forms that
 *           address, so the run is the only in-target reference.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBAndDecrementWorkByteAByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic09703_801EF488(void) {
  u8 **slots;

  slots = SPAD_PTR_TABLE(u8);
  slots[0x11][0x0B] += 1;
  slots[0x11][0x0A] -= 2;
  if (slots[0x11][0x0A] == 0) {
    u8 *record;

    record = D_801EB4E0;
    record[0x0B] -= 1;
    func_801E5988();
  }
}
