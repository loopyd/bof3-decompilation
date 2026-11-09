#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell, shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044 as g_battle_work, the name
 * this target's other lifts already use. The cell is read once here, before the
 * byte store through the loaded pointer, so the single
 * `lui $v0,0x1F80` / `lw $v0,0x44($v0)` pair covers the whole body. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, so the name needs no map row, and
 * every access here is byte-addressed, so the local declaration keeps the byte
 * view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF2AC
 * @behavior Per-frame handler reached through the pointer run at 0x801F04F8
 *           (this target's textbin entry T_801F04F8): the little-endian word
 *           0x801EF2AC at 0x801F1914 is the only place in the payload that
 *           addresses this function, so it has no jal caller. It reads the
 *           scratchpad work object through the pointer cell at 0x1F800044 once
 *           and subtracts two from the object's byte 0x0A at byte width
 *           (lbu / addiu -0x2 / sb); the store executes on both paths because
 *           the countdown is decremented in the tested expression. The compared
 *           value is the stored byte truncated to eight bits (andi
 *           $v1,$v1,0xFF) against the constant eight (addiu $v0,$zero,0x8),
 *           so the truncated register rather than a cell reload supplies it,
 *           and only on equality does the body run: it reads the shared battle
 *           work record pointer at 0x801EB4E0, decrements that record's byte
 *           0x0B by one at byte width and invokes the shared helper at
 *           0x801E5988, the store filling the call delay slot. The cell load
 *           precedes frame creation because the decrement needs no stack; the
 *           0x18-byte frame only keeps $ra at 0x10 across the call. Takes no
 *           arguments, returns nothing through the jr $ra at 0x801EF304 and
 *           reads or writes no other state. The roles of bytes 0x0A and 0x0B
 *           are not established, so both stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenEightBmagicMagic06903_801EF2AC(
    void) {
  if ((g_battle_work[0x0A] -= 2) == 8) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
