#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This target's map binds it at
 * 0x1F800044 as g_battle_work, the spelling this target's other lifts use. The
 * cell is re-evaluated for every access group because a store through the
 * loaded pointer may change the cell itself, so the compiler re-materializes
 * `lui $vN,0x1F80` / `lw $vN,0x44($vN)` exactly where the original does. The
 * first load precedes frame creation because the tested byte needs no stack. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map binds the address because the
 * overlay's remaining assembly handlers load it too, so the name needs no map
 * row and every access here is byte-addressed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Main-executable helper at 0x8015D71C; this overlay passes the selector that
 * re-arms the work-byte-9 countdown. */
void func_8015D71C(s32 arg0);

/* @source 0x801EF1D0
 * @behavior Work-byte-1 state 6 handler, entry 6 (word 0x801F0510) of the
 *           sixteen-word code-pointer run at 0x801F04F8 that the dispatcher at
 *           0x801EEE90 indexes with work byte 1. It reads the scratchpad work
 *           object through the pointer cell at 0x1F800044 and branches on the
 *           object's byte 0x0B. When that byte is nonzero it adds four to
 *           unsigned byte 0x09 (lbu / addiu 4 / sb) and, only when the stored
 *           byte truncated to eight bits reached 0x10, re-reads the cell and
 *           clears byte 0x0B. When byte 0x0B is zero it subtracts two from byte
 *           0x09 the same way and, only when the stored byte truncated to eight
 *           bits reached zero, re-reads the cell and sets byte 0x0B to one. So
 *           byte 0x0B is the direction flag of a ramp that walks byte 0x09 up by
 *           four to 0x10 and down by two to zero. Both arms then join: it loads
 *           the shared battle record pointer at 0x801EB4E0 and, only when that
 *           record's byte 0x0B equals one, invokes the main executable helper at
 *           0x8015D71C with the selector 0x101, stores 0x10 into work byte 0x09
 *           and increments work byte 0x02, advancing the byte-2 dispatch state.
 *           Every store through the loaded pointer re-reads the cell afterwards
 *           because the cell itself may change. The 0x18-byte frame only keeps
 *           $ra at 0x10 across the call; the routine takes no arguments, returns
 *           nothing through the jr $ra at 0x801EF2A4 and touches no other state.
 *           The roles of work bytes 0x09, 0x0B and 0x02 beyond this ramp,
 *           direction flag and dispatch index are not established, so they stay
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void oscillateWorkByte9UsingWorkByteBDirectionThenInvokeHelperAt101SetByte9To10AndAdvanceWorkByte2WhenGlobalWorkByteBIsOneBmagicMagic06903_801EF1D0(void) {
  if (g_battle_work[0x0B] != 0) {
    if ((g_battle_work[0x09] += 4) == 0x10) {
      g_battle_work[0x0B] = 0;
    }
  } else if ((g_battle_work[0x09] -= 2) == 0) {
    g_battle_work[0x0B] = 1;
  }
  if (D_801EB4E0[0x0B] == 1) {
    func_8015D71C(0x101);
    g_battle_work[0x09] = 0x10;
    g_battle_work[0x02] += 1;
  }
}
