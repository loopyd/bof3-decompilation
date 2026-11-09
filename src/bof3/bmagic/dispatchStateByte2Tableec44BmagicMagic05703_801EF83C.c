#include "bof3/bof3.h"

typedef void (*BmagicMagic05703Handler)(void);

typedef struct BmagicMagic05703StateTable {
  BmagicMagic05703Handler handlers[3];
} BmagicMagic05703StateTable;

/*
 * Code pointer table at 0x801EEC44, the three-entry work-state table this
 * dispatcher copies: entry 0 = 0x801EF900, entry 1 = 0x801EFA38, entry 2 =
 * 0x801EFABC. It holds the three words that follow the state-1 handler frame
 * pair in the table run at 0x801EEC20, so the symbol is bound at the word the
 * scaled index is added to rather than at the run base.
 */
extern BmagicMagic05703StateTable D_801EEC44; /* @source 0x801EEC44 @kind data */

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target map binds it at 0x1F800044 as g_battle_work. The
 * original re-materialises the cell for the dispatch index and again after the
 * indirect call, so it is read through the named target symbol instead of the
 * constant-address accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer cell in main RAM, outside this overlay image.
 * Splat binds the address through this target generated undefined_syms_auto.txt
 * row D_801EB4E0 = 0x801EB4E0, so no target-map row is needed. This body reads
 * only its byte 0x0A and leaves that offset anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Global the overlay step rewrites around a handler call; nothing in this
 * payload reads the two literals back, so the cell keeps its address-anchored
 * name. Each literal reaches it through its own lui/ori pair and its own
 * lui %hi / sw %lo store. */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header (see the note in
 * include/bof3/ui/game00_internal.h); this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x801EF83C
 * @behavior State-2 handler of the overlay work dispatcher func_801EF02C,
 *           entry 2 of the three-entry code pointer table at 0x801EEC20, so no
 *           jal in the image names this span. It publishes 0x800C3800 into the
 *           global at 0x801459F0, then copies the three words at 0x801EEC44
 *           (0x801EF900, 0x801EFA38, 0x801EFABC) into its own 0x28-byte frame
 *           and invokes the entry the scratchpad work object's unsigned byte 2
 *           selects, with no arguments; the index is not bounds-checked.
 *           After the handler returns it re-reads the work-object pointer cell
 *           at 0x1F800044 and, when work byte 0 has bit 0 set and the shared
 *           record's byte 0x0A at 0x801EB4E0 is zero, runs the shared
 *           main-executable service func_8014D290. It then publishes
 *           0x800D3800 into 0x801459F0; that literal is re-materialised in both
 *           guard delay slots and again after the service call, and the
 *           second literal is read back as the value the final ori completes.
 *           It takes no arguments and returns nothing through the jr $ra at
 *           0x801EF8F8 inside a 0x28-byte frame that keeps $ra at 0x20.
 *           Work byte 2 stays the dispatch index and record byte 0x0A stays
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Tableec44BmagicMagic05703_801EF83C(void) {
  BmagicMagic05703StateTable handlers;
  u8 *work;

  handlers = D_801EEC44;
  D_801459F0 = 0x800C3800;
  handlers.handlers[g_battle_work[2]]();
  work = g_battle_work;
  if ((work[0] & 1) != 0 && D_801EB4E0[0xA] == 0) {
    func_8014D290();
  }
  D_801459F0 = 0x800D3800;
}
