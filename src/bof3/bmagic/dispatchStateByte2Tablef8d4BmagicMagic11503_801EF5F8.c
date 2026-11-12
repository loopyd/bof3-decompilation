#include "bof3/bof3.h"

/*
 * Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map.
 * It is the named cell symbol this target's exact lifts already use, so the
 * symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,0x44($vN)). The
 * original re-reads the cell after the indirect call because the called handler
 * may rewrite it, so one shared local cannot span the call.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Global the overlay step rewrites around a handler call; it is outside this
 * payload and this body publishes only the two literals, so the cell keeps its
 * address-anchored name. */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

/*
 * Code-pointer run at 0x801EF8D4: the handler sub-table this step indexes, four
 * words into the ten-word run that starts at 0x801EF8BC. That run's word 0 holds
 * the byte-2 dispatcher 0x801EF098, word 1 holds this function's own address and
 * words 2-9 are the eight handlers 0x801EF130, 0x801EF27C, 0x801EF49C,
 * 0x801EF5C4, 0x801EF690, 0x801EF7A4, 0x801EF7F4, 0x801EF888, so this base
 * selects the last four of them. The symbol is bound at the word the scaled index
 * is added to rather than at the run base that the byte-1 dispatcher at
 * 0x801EF054 indexes.
 */
extern void (*D_801EF8D4[])(void); /* @source 0x801EF8D4 @kind data */

/* Shared main-executable service of the main executable; this caller passes no
 * arguments and discards nothing. */
extern void func_8014D290(void); /* @source 0x8014D290 */

/* @source 0x801EF5F8
 * @behavior Step wrapper around one overlay work handler: publishes 0x800C3800
 *           into the global at 0x801459F0, reads the work object published at
 *           the scratchpad pointer cell 0x1F800044, scales that object's
 *           unsigned byte 2 by four and invokes the selected entry of the
 *           code-pointer run based at 0x801EF8D4 with no arguments. After the
 *           handler returns it re-reads the pointer cell and, when the work
 *           object's byte 0 and byte 2 are both non-zero, runs the shared
 *           main-executable service func_8014D290. It then publishes 0x800D3800
 *           into 0x801459F0. Each literal reaches the cell through its own
 *           lui/ori pair and a lui %hi / sw %lo store, the second literal being
 *           re-materialised in both guard delay slots and again after the
 *           service call. The 0x18-byte frame only keeps $ra across the two
 *           calls; the body takes no arguments, returns nothing and reads or
 *           writes no other state. The 0x98-byte span equals the 0x98-byte body
 *           of dispatchStateByte2Tablef8c4BmagicMagic11503_801EF098 in every
 *           word except the %lo immediate of the table load (0x801EF8D4 here,
 *           0x801EF8C4 there), and a search for these 152 bytes over the 396
 *           payloads under out/binaries returns only this target's own copy at
 *           offset 0x9F8, so no clone seed exists and the statement order comes
 *           from that shape evidence.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Tablef8d4BmagicMagic11503_801EF5F8(void) {
  u8 *work;

  D_801459F0 = 0x800C3800;
  D_801EF8D4[g_battle_work[2]]();
  work = g_battle_work;
  if (work[0] != 0 && work[2] != 0) {
    func_8014D290();
  }
  D_801459F0 = 0x800D3800;
}
