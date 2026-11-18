#include "bof3/battle/battle15_internal.h"

/* Four-entry table of byte-table pointers used by the packed selection-id
 * lookups (D_800B6AB0[selector >> 8][selector & 0xFF]). */
extern u8 *D_800B6AB0[]; /* @source 0x800B6AB0 @kind table */

/* @source 0x800AE260
 * @behavior Stores the packed selection id in the selection-kind halfword
 * D_801463C0, resolves the grouped byte at D_800B6AB0[selector >> 8][selector & 0xFF],
 * claims a queued slot of mode 2 carrying that byte through func_801E590C, and
 * publishes the second argument into the claimed slot's +0x74 pointer word.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSelectedKindQueueSlotFromGroupedTableAndPublishPointer(u32 selector, u32 value) {
  u32 slot;
  u16 id;

  id = selector;
  D_801463C0 = id;
  slot = func_801E590C(2u, D_800B6AB0[(id >> 8) & 0xFF][selector & 0xFF]) & 0xffu;
  *(u32*)&D_801EC3A4[slot * 0x78] = value;
}
