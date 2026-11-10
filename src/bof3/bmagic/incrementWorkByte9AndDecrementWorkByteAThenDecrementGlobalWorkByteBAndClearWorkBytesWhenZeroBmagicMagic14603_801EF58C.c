#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map as
 * g_battle_work, the name its exact lift
 * clearWorkBytes0Through4BmagicMagic14603_801EF964 uses. The cell is
 * re-evaluated for each access group because the byte store that precedes the
 * second group goes through the pointer the cell supplied, so the original
 * re-materializes `lui $v1,0x1F80` / `lw $v1,0x44($v1)` exactly twice, at
 * 0x801EF58C and 0x801EF5AC. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because the overlay's remaining assembly spans load it, so the name needs no
 * map row, and every access here is byte-addressed, so the local declaration
 * keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Work-byte clear routine at 0x801EF964, bound by this target's map. Its own
 * 80-byte lift measured exact against the payload. */
extern void clearWorkBytes0Through4BmagicMagic14603_801EF964(void); /* @source 0x801EF964 @kind function */

/* @source 0x801EF58C
 * @behavior Per-frame handler of the battle work object published at the
 *           scratchpad pointer cell 0x1F800044. It increments that object's
 *           unsigned byte 0x09 by one at byte width (lbu / addiu 1 / sb) and
 *           does not test the sum, re-reads the cell and subtracts one from
 *           unsigned byte 0x0A the same way, then tests the stored byte
 *           truncated to eight bits (andi $v0,$v0,0xFF / bnez), so the
 *           truncated register - not a reload - supplies the compared value.
 *           Only when byte 0x0A reaches zero does it read the shared battle
 *           work record pointer at 0x801EB4E0, decrement that record's byte
 *           0x0B by one at byte width and call this overlay's work-byte clear
 *           routine at 0x801EF964, the store filling the call delay slot. The
 *           first cell load precedes frame creation because the increment
 *           needs no stack; the 0x18-byte frame only keeps $ra at 0x10 across
 *           the call. Takes no arguments, returns nothing through the jr $ra
 *           at 0x801EF5FC and reads or writes no other state, so the roles of
 *           work bytes 9, 0xA and 0xB stay offset-anchored. No jal in this
 *           payload reaches it: its address is the last word of the
 *           seven-word code-pointer run based at 0x801EF9B4, which the byte-1
 *           dispatcher dispatchStateByte1Tablef9b4BmagicMagic14603_801EF04C
 *           indexes, so the table word at 0x801EF9CC is its only in-payload
 *           reference. A cmp of the 120-byte span at payload offset 0x98C
 *           against emi/bmagic/magic059/03 0x801F012C, emi/bmagic/magic059/03
 *           0x801EF5C0, emi/bmagic/magic126/03 0x801EFCE4,
 *           emi/bmagic/magic145/03 0x801EE0A0 and emi/bmagic/magic009/03
 *           0x801EF340 differed in the call word at +0x60 alone, each naming
 *           its own overlay's helper, so no byte-identical source exists for
 *           the clone lever and the body was transcribed from the exact
 *           sibling shape at emi/bmagic/magic059/03 0x801F012C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndClearWorkBytesWhenZeroBmagicMagic14603_801EF58C(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  work[0x09] += 1;
  work = g_battle_work;
  counter = work[0x0A] - 1;
  work[0x0A] = counter;
  if (counter == 0) {
    D_801EB4E0[0x0B] -= 1;
    clearWorkBytes0Through4BmagicMagic14603_801EF964();
  }
}
