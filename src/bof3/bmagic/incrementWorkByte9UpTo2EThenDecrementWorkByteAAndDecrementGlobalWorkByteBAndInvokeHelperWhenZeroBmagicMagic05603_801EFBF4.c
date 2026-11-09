#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * address (lui $vN,0x1F80 / lw $vN,68($vN)) for both of its reads, and each
 * store goes through the pointer the cell supplied, so every access group
 * spells the named cell again instead of reusing the first loaded pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because the remaining assembly spans load it, so the name needs no map row,
 * and every access here is byte-addressed, so the local declaration keeps the
 * byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFBF4
 * @behavior Third entry of the byte-0x9 timer sequence reached through word 2
 *           of the in-image code-pointer run at 0x801F0764, dispatched from
 *           outside this overlay. It reads the scratchpad work-object pointer
 *           cell at 0x1F800044 once, compares that object's unsigned byte 9
 *           truncated with andi 0xFF against 0x2E, and only when the byte is
 *           not already 0x2E stores the value widened by one at byte width
 *           (addiu 1 filling the branch delay slot, then sb), so the counter
 *           saturates at 0x2E. It then re-reads the cell and subtracts one from
 *           the object's unsigned byte 0xA at byte width, testing that stored
 *           register truncated with andi 0xFF (bnez) instead of reloading the
 *           cell. Only when that countdown reaches zero does it read the shared
 *           battle work record pointer at 0x801EB4E0, decrement that record's
 *           byte 0xB by one at byte width and invoke the shared helper at
 *           0x801E5988, the store filling the call delay slot. The first cell
 *           load precedes frame creation because the increment needs no stack;
 *           the 0x18-byte frame only keeps $ra at 0x10 across the call. Takes no
 *           arguments, returns nothing and reads or writes no other state. The
 *           128-byte span is byte-identical to the still-unlifted asm span
 *           func_801F0460 of emi/bmagic/magic103/03, so only this payload's own
 *           address names differ from that target's span and no source exists
 *           to clone from. The body contains two relocations, the 0x801EB4E0
 *           address constant and the 0x801E5988 call, both already bound by
 *           this target's generated auto files.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9UpTo2EThenDecrementWorkByteAAndDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic05603_801EFBF4(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[0x09];
  if (counter != 0x2E) {
    work[0x09] = counter + 1;
  }
  if (--g_battle_work[0x0A] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
