#include "bof3/bof3.h"

/* Work-record pointer cell in the shared scratchpad. It lies outside every
 * overlay payload and this target's map binds no name for it, so it keeps the
 * raw address spelling that this target's exact lifts of the same cell use. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared record-submit helper of the main executable, outside this payload. */
extern void func_80196718(void *record); /* @source 0x80196718 */

/* @source 0x801F35D4
 * @behavior Clears byte 0x48 of the work record published through the
 *           shared-scratchpad pointer cell 0x1F800044, then reloads that cell
 *           and hands the record to the shared record-submit helper
 *           func_80196718. The original loads the record through the cell
 *           before it creates the 0x18-byte frame, that frame only keeps $ra at
 *           0x10 across the call, and the cell is reloaded for the argument
 *           because the byte store may reach the cell word itself. The call
 *           delay slot is a nop. The aligned pointer word at 0x801F4EF8
 *           names this entry inside the run beginning at 0x801F4EF0 in the
 *           D_801F409C data blob. The body takes no arguments and returns
 *           nothing. The role of record byte 0x48 is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Evidence: the span 0x801F35D4-0x801F3607 is one complete 0x34-byte function
 * that fills its Splat row exactly, with one `addiu $sp,-0x18` frame and one
 * `jr $ra` epilogue, so it is neither data nor a fragment. The original words
 * read `lui $v0,0x1F80` / `lw $v0,0x44($v0)` / `addiu $sp,$sp,-0x18` /
 * `sw $ra,0x10($sp)` / `sb $zero,0x48($v0)` / `lui $a0,0x1F80` /
 * `lw $a0,0x44($a0)` / `jal 0x80196718` with a `nop` delay slot, so the stored
 * byte offset is 0x48 and the submitted argument is the cell's own reload. A
 * scan of the 0x2370-byte payload finds the aligned word 0x801F35D4 at
 * offset 0x22F8, runtime address 0x801F4EF8. Its neighboring words at
 * 0x801F4EF4 and 0x801F4EFC name 0x801F34FC and 0x801F3608.
 *
 * Name evidence: two whole-body twins under this behaviour are already lifted
 * with the name clearWorkByte48ThenSubmitRecordWorld<T>Area<A><13>_<ADDRESS>,
 * and this span is byte-identical to both: a `cmp` of the 52-byte spans at
 * payload offset 0x9D4 of emi/world02/area100/13.bin, 0xA38 of
 * emi/world02/area112/13.bin and 0x1040 of emi/world03/area143/13.bin leaves
 * every byte equal. So the byte store at 0x48, the cell reload and the submit
 * call of the name are this span's own words, not a copied claim.
 *
 * Shape evidence: statement order, operand forms and declaration style follow
 * clearWorkByte48ThenSubmitRecordWorld02Area11213_801F3638, whose own gate
 * measured exact on the same span, with this function's own literals and cell
 * bindings written from the words above. The pointer-run location above was
 * verified in this payload rather than copied from the donor's comment.
 */
void stateClearSubmitRecord(void) {
  D_1F800044[0x48] = 0;
  func_80196718(D_1F800044);
}
