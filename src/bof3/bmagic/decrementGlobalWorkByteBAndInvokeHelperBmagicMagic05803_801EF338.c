#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle record pointer published by the main executable at 0x801EB4E0,
 * outside this overlay image and already bound by this target's generated
 * undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0, so the name needs no
 * target-map row. The body reads it once and touches only byte 0x0B of the
 * record it names, which stays a byte view of that pointer. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF338
 * @behavior Word 5 of the nine-word stage run the payload header holds at
 *           0x801EEC04: the word at 0x801EEC18 is 0x801EF338 and it is the only
 *           word in this payload that names this address, since no jal reaches
 *           it and no other pointer word holds it. The per-frame dispatcher at
 *           0x801EEC28 copies that run into its own frame and calls the entry
 *           the work object's unsigned byte 0x01 selects, with no bound on the
 *           index, so this body runs as one stage of the overlay's work byte 0x01
 *           chain. It reads the shared battle record pointer at 0x801EB4E0 once,
 *           before frame creation because the decrement needs no stack, decrements
 *           that record's byte 0x0B by one at byte width (lbu 0xB, addiu -1, then
 *           sb 0xB) and invokes the shared helper at 0x801E5988 unconditionally,
 *           the byte store filling the call's delay slot. The 0x18-byte frame
 *           only keeps $ra at 0x10 across that call. Takes no arguments, returns
 *           nothing, and reads or writes no other state. The 0x34-byte span
 *           compared equal to emi/bmagic/magic054/03 0x801EF624, an already exact
 *           lift, in one `cmp` of both payloads at offsets 0x738 and 0xA24, so its
 *           statement order and operand forms were transcribed unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperBmagicMagic05803_801EF338(void) {
  D_801EB4E0[0xB]--;
  func_801E5988();
}
