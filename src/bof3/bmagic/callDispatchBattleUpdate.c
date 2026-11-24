#include "bof3/bof3.h"

typedef void (*BmagicMagic13803Handler)(void);

typedef struct BmagicMagic13803DispatchTable {
  BmagicMagic13803Handler handlers[3];
} BmagicMagic13803DispatchTable;

/* Interior base of the overlay header handler run at 0x801EEC04: entries 3, 4
 * and 5 of the nine-word run, the words at 0x801EEC10, 0x801EEC14 and
 * 0x801EEC18 (0x801EF04C, 0x801EF3A0, 0x801EF50C). This dispatcher copies
 * exactly those three words into its own frame, so the declaration covers that
 * prefix and not the run. The run sits in the overlay's leading data block,
 * which no map row owns, so the reference binds through the target's generated
 * undefined-symbol table. */
extern BmagicMagic13803DispatchTable D_801EEC10; /* @source 0x801EEC10 @kind data */

/* Shared dispatch-context cell published around the handler call. */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-reads the cell after the handler call, so the second access
 * goes through this target-map symbol instead of a cached pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle update; its call site here passes no arguments. */
extern void func_8014D4E0(void); /* @source 0x8014D4E0 @kind function */

/* @source 0x801EEFA4
 * @behavior Copies the three code-pointer words at 0x801EEC10, entries 3, 4 and
 *           5 of the overlay header handler run at 0x801EEC04, into its own
 *           0x28-byte frame at 0x10..0x18 and publishes 0x800C3800 into the
 *           shared cell at 0x801459F0. It then reads the scratchpad work
 *           pointer at 0x1F800044 and calls the copied entry that unsigned work
 *           byte 2 selects, so the three reachable handlers are 0x801EF04C,
 *           0x801EF3A0 and 0x801EF50C. After that call it re-reads the cell and,
 *           when work byte 0 has bit 0 set, calls the shared battle update at
 *           0x8014D4E0, then publishes 0x800D3800 into 0x801459F0. It takes no
 *           arguments and returns nothing; the frame only keeps $ra at 0x20
 *           across the handler call, and the two values published at 0x801459F0
 *           are the only state this body writes besides that call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callDispatchBattleUpdate(void) {
  BmagicMagic13803DispatchTable handlers;
  u8 *work;

  handlers = D_801EEC10;
  D_801459F0 = 0x800C3800;
  handlers.handlers[g_battle_work[2]]();
  work = g_battle_work;
  if ((work[0] & 1) != 0) {
    func_8014D4E0();
  }
  D_801459F0 = 0x800D3800;
}
