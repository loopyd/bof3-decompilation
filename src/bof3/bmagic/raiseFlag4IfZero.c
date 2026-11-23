#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEE58
 * @behavior Work-byte-1 handler, entry 1 of the two-entry handler array that
 *           0x801EEC04 seeds in its own frame, selected by the scratchpad work
 *           object's unsigned byte at offset 1; no word of this payload stores
 *           this address, so the dispatcher's immediate is the only reference.
 *           Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           and, only when the work object's byte at offset 0xB is zero, raises
 *           bit 0x4 of the shared battle global halfword at 0x801462E8 and then
 *           invokes the shared helper at 0x801E5988 with no arguments, the
 *           store filling the call delay slot. The pointer cell is loaded
 *           before the 0x18-byte frame is created, that frame only keeps $ra
 *           across the helper call. The halfword read-modify-write preserves
 *           the shared object's volatile access. Nothing is returned and no other state is read or
 *           written. The 0x4C-byte body is byte-identical to the exact lift at
 *           0x801F01E4 (MAGIC008.EMI#0), which seeded this source.
 * @status partial
 * @match 85.00
 * @residual Volatile halfword access leaves an extra instruction and a four-byte size surplus.
 */
void raiseFlag4IfZero(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0) {
    volatile u16 *flags = &D_801462E8;

    *flags |= 4;
    func_801E5988();
  }
}
