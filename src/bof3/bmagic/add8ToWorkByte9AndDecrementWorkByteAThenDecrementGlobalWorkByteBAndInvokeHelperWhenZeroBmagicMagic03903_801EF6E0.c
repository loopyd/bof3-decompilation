#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell 0x1F800044, already bound at that address
 * by this target's map as g_battle_work, the name its exact lifts
 * add16ToWorkByte9AndAdvanceWorkByte2At80BmagicMagic03903_801EF690 and
 * add2ToWorkByte9ClearWorkByteAAndAdvanceWorkByte2At10BmagicMagic03903_801EF0D4
 * use. The cell is evaluated once per access group because the byte store that
 * precedes the second group goes through the pointer the cell supplied, so the
 * compiler re-materializes `lui $v1,0x1F80` / `lw $v1,0x44($v1)` exactly twice,
 * at 0x801EF6E0 and 0x801EF700. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because the remaining assembly spans func_801EEC94, func_801EF070,
 * func_801EF44C and func_801EF584 load it, so the name needs no map row, and
 * every access here is byte-addressed, so the local declaration keeps the byte
 * view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF6E0
 * @behavior Per-frame handler entered through this target's tabledata segment:
 *           its address is the word at 0x801EEC30, entry index 11 (last) of the
 *           twelve-word code-pointer run 0x801EEC04..0x801EEC34. The only
 *           in-payload reader of that run,
 *           dispatchWorkByte1HandlerBmagicMagic03903_801EEC34, copies its first
 *           three words into a 0x28-byte frame and indexes them with work byte
 *           0x1, so this entry is not selected from this payload's own code and
 *           no jal word in the payload names 0x801EF6E0. It reads the
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
 *           the jr $ra at 0x801EF750 and reads or writes no other state. The
 *           roles of work-object bytes 9, 0xA and 0xB are not established, so
 *           all three stay offset-anchored. The 120-byte span is byte-identical
 *           to the exact lift
 *           add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteB
 *           AndInvokeHelperWhenZeroBmagicMagic00603_801F0290, whose body is
 *           transcribed here unchanged; only the own-target facts above differ.
 *           The body contains two relocations, the 0x801EB4E0 address constant
 *           and the 0x801E5988 call, both already bound by this target's
 *           generated auto files.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic03903_801EF6E0(void) {
  g_battle_work[0x09] += 8;
  if (--g_battle_work[0x0A] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
