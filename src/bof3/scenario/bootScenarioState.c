#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F9178
 * @behavior Boots state 0 of the overlay's own state machine (the entry that
 * the code-span dispatcher func_801F913C selects from the main-RAM state byte
 * D_80146872): it runs the front-end scene init func_801C1DF0(0), seeds the
 * front selector/context bundle - selection seed 0x18, the channel pair
 * 0x630000/0xC0000 and kind 3, both published in the bundle words D_801448FC/
 * D_801448FF/D_80144900/D_80144904 and handed to func_8019FA28 - clears the
 * two route work-area slot tables (6 four-byte slots at 0x80145500 and 12 at
 * 0x80145518) to 0xFF, raises flag 0x40 in the mode word D_80146258, requests
 * primary scene state 5 in D_80146874 with secondary state 0 in D_80146875,
 * yields with func_8014B87C(1) until the EMI loader reports ready through
 * func_80162D00, and finally clears the scenario progress word and advances
 * D_80146872 to 1 for the next frame. Takes no arguments and returns nothing;
 * the address is entry 0 of the overlay-local handler table at 0x801FCA10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void bootScenarioState(void) {
  Scena00RouteWork* work;
  u8 (*slot_rows)[4];
  u8 i;
  u8 j;

  func_801C1DF0(0u);
  func_8019FA28(0x18u, 0x630000u, 0xc0000u, 3u);

  /* The slot-scan counter starts before the bundle stores so the original's
   * schedule keeps its counter move ahead of the work-area address
   * materialisation; the first table keeps that counter and only the second
   * table resets it. */
  i = 0;
  work = &D_801448FC;
  work->selection_seed_00 = 0x18u;
  D_80144900 = 0x630000u;
  D_80144904 = 0xc0000u;
  D_801448FF = 3u;
  D_80146874 = 5u;
  D_80146875 = 0u;

  /* The first table hangs off the work-area address the seed store
   * materialised (0x801448FC + 0xC04); the second is its own symbol. */
  slot_rows = work->slot_flags_a_C04;
  for (; i < 6; i++) {
    for (j = 0; j < 4; j++) {
      slot_rows[i][j] = 0xFFu;
    }
  }

  for (i = 0; i < 12; i++) {
    for (j = 0; j < 4; j++) {
      D_80145518[i][j] = 0xFFu;
    }
  }

  D_80146258 |= 0x40u;

  while (!func_80162D00()) {
    func_8014B87C(1u);
  }

  g_ScenarioProgress = 0u;
  D_80146872 = 1;
}
