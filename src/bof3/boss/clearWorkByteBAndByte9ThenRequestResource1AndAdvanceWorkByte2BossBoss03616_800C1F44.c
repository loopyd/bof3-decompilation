#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the pointer run dispatcher at 0x800C1EE4
 * and every handler of this payload reach the work record through this cell.
 * Declared as the cell itself, the spelling the exact sibling
 * clearWorkByteBAndPublishClassRowByteThenRequestResource4AndAdvanceWorkByte2BossBoss05216_800C1DF8
 * uses, so each access folds to lui plus an offset load and the record pointer
 * is re-read before each later store group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Main-executable resource request helper at 0x8014D6B8: the sibling boss lifts
 * of this family declare the same prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag);

/* Shared main-program helpers this boss archive family runs after the resource
 * request; both take no arguments and this payload discards any result. */
extern void func_8014D978(void);
extern void func_8014D4E0(void);

/* @source 0x800C1F44
 * @behavior Entry 0 of this payload's three-word in-image code pointer run at
 * 0x800C1804, which holds 0x800C1F44, 0x800C1FA4 and 0x800C2000. The byte-2
 * dispatcher at 0x800C1EE4 copies that run into its own frame and indexes it
 * with the work-record byte at offset 2, so this handler is the state byte 2
 * entry 0. It takes the work-record pointer published at the scratchpad pointer
 * cell 0x1F800044 and clears that record's byte 0x0B, re-reads the cell and
 * clears record byte 0x09, requests resource 1 through the shared helper at
 * 0x8014D6B8, runs the shared battle updates 0x8014D978 and 0x8014D4E0 in that
 * order, then re-reads the cell once more and advances record byte 0x02 by one,
 * so the next dispatch on that byte enters word 1 of the same run, 0x800C1FA4.
 * The 0x18-byte non-leaf frame keeps $ra at 0x10 across the calls; the first
 * pointer-cell read precedes frame creation and each later store group re-reads
 * the cell, because a store through the loaded record pointer may alias it. No
 * jal names 0x800C1F44; its only in-image reference is that word at 0x800C1804,
 * so the pointer run selects it and this handler takes no arguments. The byte
 * 0x09 and 0x02 stores are byte stores and the increment leaves the new value in
 * $v0 while the epilogue restores only $ra.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape evidence: instruction-for-instruction the same body as the exact
 * sibling clearWorkByteBAndPublishClassRowByteThenRequestResource4AndAdvanceWorkByte2BossBoss05216_800C1DF8,
 * from the same 0x18-byte frame and the same split work-cell locals down to the
 * identical tail (lbu work byte 2, addiu one, then restore $ra before the byte
 * store). That sibling differs only in the value it stores into work byte 0x09
 * (a class-row byte there, the constant zero here) and in the resource id it
 * passes to 0x8014D6B8 (4 there, 1 here). The nearest payload twin anywhere in
 * the corpus is 0x801F001C of emi/bmagic/magic137/03, whose 0x60 bytes differ
 * from these in exactly one word, the resource id 3 versus 1; its row is still
 * asm, so it supplies the equality proof and no source to clone.
 */
void clearWorkByteBAndByte9ThenRequestResource1AndAdvanceWorkByte2BossBoss03616_800C1F44(
    void) {
  u8 *work;
  u8 *slot;
  u8 *step;

  work = D_1F800044;
  work[0xB] = 0;
  slot = D_1F800044;
  slot[0x9] = 0;
  func_8014D6B8(1);
  func_8014D978();
  func_8014D4E0();
  step = D_1F800044;
  step[0x2]++;
}
