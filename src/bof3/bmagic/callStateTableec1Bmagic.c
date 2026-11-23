#include "bof3/bof3.h"

typedef void (*BmagicMagic13803Handler)(void);

typedef struct BmagicMagic13803DispatchTable {
  BmagicMagic13803Handler handlers[3];
} BmagicMagic13803DispatchTable;

/* Interior base of the overlay header handler run at 0x801EEC04: entries 6, 7
 * and 8 of the nine-word run (0x801EF5D0, 0x801EF6A4, 0x801EF6F8). This
 * dispatcher copies exactly those three words into its own frame, so the
 * declaration covers that prefix and not the run. The run sits in the overlay's
 * leading data block, which no map row owns, so the reference binds through the
 * target's generated undefined-symbol table. */
extern BmagicMagic13803DispatchTable D_801EEC1C; /* @source 0x801EEC1C @kind data */

/* Shared dispatch-context cell published around the handler call. */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-reads the cell after the handler call, so the second access
 * goes through this target-map symbol instead of a cached pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* This call site passes no arguments to the shared main-executable service. */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801EF52C
 * @behavior Copies the three code-pointer words at 0x801EEC1C, entries 6, 7 and
 *           8 of the overlay header handler run at 0x801EEC04, into its own
 *           0x28-byte frame at 0x10..0x18 and publishes 0x800F0800 into the
 *           shared cell at 0x801459F0. It then reads the scratchpad work
 *           pointer at 0x1F800044 and calls the copied entry that unsigned work
 *           byte 2 selects, so the three reachable handlers are 0x801EF5D0,
 *           0x801EF6A4 and 0x801EF6F8. After that call it re-reads the cell and,
 *           when work byte 0 reads nonzero, calls the shared service at
 *           0x8014D290, then publishes 0x800D3800 into 0x801459F0. It takes no
 *           arguments and returns nothing; the frame only keeps $ra at 0x20
 *           across the handler call, and the value published at 0x801459F0 is
 *           the only state this body writes besides that call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callStateTableec1Bmagic(void) {
  BmagicMagic13803DispatchTable handlers;
  u8 *work;

  handlers = D_801EEC1C;
  D_801459F0 = 0x800F0800;
  handlers.handlers[g_battle_work[2]]();
  work = g_battle_work;
  if (work[0] != 0) {
    func_8014D290();
  }
  D_801459F0 = 0x800D3800;
}
