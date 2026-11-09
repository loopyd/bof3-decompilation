#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell, shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044 as g_battle_work, the name
 * its exact lifts incrementWorkByte2WhenGlobalWorkByteBIs1Bmagic
 * Magic00603_801F0644 and decrementWorkByte9ThenDecrementGlobalWorkByteBAnd
 * InvokeHelperWhenZeroBmagicMagic00603_801EF41C already use. The cell is
 * evaluated for each access group because the byte store through the loaded
 * pointer may alias it, so the compiler re-materializes the
 * `lui $vN,0x1F80` / `lw $vN,0x44($vN)` pair exactly where the original does
 * (twice in this body). */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because the remaining assembly spans func_801EEC90, func_801EEF28,
 * func_801EF298, func_801F0290 and func_801F0684 load it, so the name needs no
 * map row, and every access here is byte-addressed, so the local declaration
 * keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0290
 * @behavior Per-frame handler selected by the scratchpad work object's
 *           dispatch byte 0x2: entry index 8 of the thirteen-word code-pointer
 *           run 0x801EEC04..0x801EEC34 in this target's tabledata segment, the
 *           table func_801EEE50 copies to its stack frame and indexes with
 *           `lbu 0x2($v0)` before `jalr`. The word at 0x801EEC24 holds this
 *           address and is the only word in the payload that does, so the entry
 *           is reached through that table and has no jal caller. It reads the
 *           scratchpad work object through the pointer cell at 0x1F800044 and
 *           adds 8 to its unsigned byte 9 at byte width (lbu / addiu 8 / sb);
 *           the sum is not tested, so the register is stored without a
 *           following andi. It then re-reads the cell and subtracts one from
 *           unsigned byte 0x0A the same way, testing the stored byte truncated
 *           to eight bits (andi $v0,$v0,0xFF / bnez), so the truncated register
 *           rather than a reload supplies the compared value, and the rest of
 *           the body runs only when that countdown reaches zero. On zero it
 *           reads the shared battle work record pointer at 0x801EB4E0,
 *           decrements that record's byte 0x0B by one at byte width and invokes
 *           the shared helper at 0x801E5988, the store filling the call delay
 *           slot. The first cell load precedes frame creation because the
 *           increment needs no stack; the 0x18-byte frame only keeps $ra at
 *           0x10 across the call. Takes no arguments, returns nothing through
 *           the jr $ra at 0x801F0300 and reads or writes no other state. The
 *           roles of work-object bytes 9, 0xA and 0xB are not established, so
 *           all three stay offset-anchored. The 120-byte span is byte-identical
 *           to the still-asm row of emi/bmagic/magic039/03 (0x801EF6E0), which
 *           has no C yet, so the clone lever offered no source; it differs from
 *           the reviewed exact lift
 *           incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteB
 *           AndInvokeHelperWhenZeroBmagicMagic14503_801EF0A0 in exactly one
 *           byte, the low byte of the addiu immediate (1 versus 8), so the
 *           statement form, cell access grouping and register choice are taken
 *           from that twin unchanged. The body contains two relocations, the
 *           0x801EB4E0 address constant and the 0x801E5988 call, both already
 *           bound by this target's generated auto files.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic00603_801F0290(void) {
  g_battle_work[0x09] += 8;
  if (--g_battle_work[0x0A] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
