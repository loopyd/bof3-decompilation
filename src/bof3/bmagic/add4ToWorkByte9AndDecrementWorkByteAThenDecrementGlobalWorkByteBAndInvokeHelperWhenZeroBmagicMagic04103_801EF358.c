#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map as g_battle_work, the
 * name the lifted siblings on this target use. It is written as the named cell
 * symbol rather than an integer constant: the symbolic address lets the
 * assembler fold it into each load (lui $v1,0x1F80 / lw $v1,68($v1)), and the
 * original takes the cell twice, at 0x801EF358 and again at 0x801EF378, because
 * the byte store through the pointer the first load supplied may alias the cell
 * itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * (the assembly spans func_801EF3D0 and func_801EF670 load it), so the name
 * needs no map row, and every access here is byte-addressed, so the local
 * declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF358
 * @behavior Per-frame handler entered through this target's tabledata segment:
 *           its address is the word at 0x801EEC24, entry index 8 of the
 *           twelve-word code-pointer run 0x801EEC04..0x801EEC30, which this
 *           payload's dispatcher copies and indexes, so no jal word names it.
 *           It reads the scratchpad work object through the pointer cell at
 *           0x1F800044 and adds four to that object's unsigned byte 9 at byte
 *           width (lbu / addiu 4 / sb); the sum is not tested, so the register
 *           is stored without a following andi and the increment needs no
 *           stack, which is why the first cell load precedes frame creation.
 *           It then re-reads the cell and subtracts one from unsigned byte 0x0A
 *           the same way, testing the stored byte truncated to eight bits
 *           (andi $v0,$v0,0xFF / bnez), so the truncated register rather than a
 *           reload supplies the compared value; the rest of the body runs only
 *           when that countdown reaches zero.
 *           On zero it reads the shared battle work record pointer at
 *           0x801EB4E0, decrements that record's byte 0x0B by one at byte width
 *           and invokes the shared helper at 0x801E5988, the store filling the
 *           call delay slot. The 0x18-byte frame only keeps $ra at 0x10 across
 *           that call. Takes no arguments, returns nothing through the jr $ra
 *           at 0x801EF3C8 and reads or writes no other state. The roles of
 *           work-object bytes 9, 0xA and 0xB are not established, so all three
 *           stay offset-anchored.
 *           Shape: the exact lift
 *           add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteB
 *           AndInvokeHelperWhenZeroBmagicMagic03903_801EF6E0 carries the same
 *           0x78-byte body and differs only in the added constant (0x8 there,
 *           0x4 here) and its branch relocation, so this body is its
 *           transcription with that constant re-derived from the original
 *           bytes; the byte difference makes it shape evidence, not a clone.
 *           The cell re-read, the stored-then-masked byte test and the store in
 *           the call delay slot all come from that sibling's byte-for-byte
 *           agreement. The two addresses 0x801EB4E0 and 0x801E5988 are already
 *           bound by this target's generated auto files.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add4ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic04103_801EF358(void) {
  g_battle_work[0x09] += 4;
  if (--g_battle_work[0x0A] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
