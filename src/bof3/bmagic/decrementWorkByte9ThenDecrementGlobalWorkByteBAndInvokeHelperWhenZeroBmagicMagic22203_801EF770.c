#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. This body reads the
 * cell once, so the single access stays one plain pointer load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind unknown */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, so the name needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF770
 * @behavior Work-byte-1 handler, entry 13 of the sixteen-word handler-pointer
 *           run at 0x801F0D54 whose word at 0x801F0D88 holds this function's
 *           address, the only word in the payload that addresses 0x801EF770.
 *           Reads the scratchpad work object through the pointer cell at
 *           0x1F800044 and decrements its unsigned byte 9 in place (lbu /
 *           addiu -1 / sb / andi 0xFF / bnez), testing the stored byte
 *           truncated to eight bits rather than a reload, so the rest of the
 *           body runs only when that countdown reaches zero. On zero it reads
 *           the shared battle work record pointer at 0x801EB4E0, decrements
 *           that record's byte 0xB by one at byte width and invokes the shared
 *           helper at 0x801E5988, the store filling the call delay slot. The
 *           pointer-cell load precedes frame creation because the countdown
 *           needs no stack; the 0x18-byte frame only keeps $ra at 0x10 across
 *           the call. Takes no arguments, returns nothing and reads or writes
 *           no other state. The 92-byte span is byte-for-byte identical to the
 *           exact sibling decrementWorkByte9ThenDecrementGlobalWorkByteBAnd
 *           InvokeHelperWhenZeroBmagicMagic13103_801F02B0 (compared on this
 *           payload and that target's payload), so this body was seeded by the
 *           clone lever from that source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic22203_801EF770(void) {
  if (--g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B]--;
    func_801E5988();
  }
}
