#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's step handlers.
 * Kept as the named cell symbol bound in this target's map instead of the
 * constant-address macro: the symbolic address folds into each load
 * (lui $vN,0x1F80 / lw $vN,68($vN)), so each of this body's three access groups
 * re-materialises the base on its own, which is what the original emits at
 * 0x801EF3C4, 0x801EF3E4 and 0x801EF400. A byte store through the pointer may
 * alias the cell, so every group is a fresh read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image; Splat
 * binds the address through this target's generated undefined_syms_auto.txt,
 * so this lift needs no target-map row. The family keeps the role
 * address-anchored: the exact sibling at 0x801EF270 writes byte 0xB of the same
 * record, and nothing in this payload names the record. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared helper in the main executable, outside this overlay's image; the
 * target's generated undefined-funcs map binds the address. */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF3C4
 * @behavior Frame step of the overlay work object, entry 6 of this overlay's
 *           handler-pointer run based at 0x801F0CA8: its address is the word at
 *           0x801F0CC0, and no jal word and no other word in the payload names
 *           it. It reads the scratchpad work-object pointer cell at 0x1F800044
 *           and increments the object's unsigned byte 0xB by one at byte width
 *           (lbu / addiu 1 / sb), then re-reads the cell and subtracts one from
 *           unsigned byte 0x4 the same way (lbu / addiu -1 / sb), then re-reads
 *           the cell a third time, increments unsigned byte 0x9 and tests the
 *           truncated value it just stored (andi 0xFF of the stored register,
 *           not a second load of the byte). Only when that stored byte reaches
 *           0x88 does it read the shared battle work pointer at 0x801EB4E0,
 *           store 0xFF into that record's byte 0xB and invoke the shared helper
 *           at 0x801E5988; the 0xFF constant materialises in the branch delay
 *           slot and the record store fills the call delay slot. The first cell
 *           load precedes frame creation because the increment needs no stack;
 *           the 0x18-byte frame only keeps $ra at 0x10 across the call. Takes
 *           no arguments, returns nothing through the jr $ra at 0x801EF444 and
 *           reads or writes no other state. The roles of work-object bytes 0xB,
 *           0x4 and 0x9 and of the global work byte B it raises stay
 *           offset-anchored.
 *
 *           Statement order, operand forms, offset spelling and declaration
 *           style follow three exact siblings: 0x801EF270, which makes the same
 *           leading cell groups and the same global byte B store, 0x801EF374,
 *           which carries the truncated byte-9 compare against a materialised
 *           literal, and the magic097/03 entry at 0x801EF488, whose record
 *           store fills the helper call delay slot. This 136-byte span is
 *           byte-identical to the still-asm row emi/bmagic/magic122/03
 *           0x801EF29C, proven with one payload comparison, whose own row has
 *           no source to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementStepSetGlobal(void) {
  u8 *record;

  g_battle_work[0x0B] += 1;
  g_battle_work[0x04] -= 1;
  if ((g_battle_work[0x09] += 1) == 0x88) {
    record = D_801EB4E0;
    record[0x0B] = 0xFF;
    func_801E5988();
  }
}
