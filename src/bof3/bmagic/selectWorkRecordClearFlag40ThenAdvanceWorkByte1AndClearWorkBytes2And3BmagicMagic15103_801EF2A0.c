#include "bof3/bof3.h"

/* Local work-record table in main RAM, outside this overlay's image. The
 * 0x140-byte-stride record array that the front end and the battle overlays
 * select by a global byte; the exact game00 lift func_801C0F9C clears the same
 * byte 0 bit through D_80145E90[offset] &= 0xBF, and the exact world lift
 * func_801F363C_area03213 publishes the table base at the scratchpad cell.
 * Splat binds the address through this target's generated undefined_syms_auto
 * .txt, so no target-map row is added here. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */

/* Battle selection byte in main RAM, outside this overlay's image. The battle
 * targets own the address; this lift only reads it as the work-record
 * selector. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Scratchpad work-object pointer cell, already bound by this target's map row
 * g_battle_work = 0x1F800044. Declared as a named symbol rather than through
 * SPAD_PTR_SLOT so each access re-materialises the cell address with its own
 * lui, which is what the original does before every load or store of the cell;
 * the bare constant address is instead hoisted once into a base register. The
 * exact siblings add4ToWorkByte9AndAdvanceWorkByte2At40BmagicMagic13103_801F053C
 * and func_801F363C_area03213 declare the same cell this way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF2A0
 * @behavior Selects the 0x140-byte-stride local work record at the battle
 *           selection byte D_80146374, publishes that record at the scratchpad
 *           work-object pointer cell 0x1F800044, and clears bit 0x40 of the
 *           record's byte 0 through the record address. It then restores the
 *           previously published work object and resets it: byte 1 is
 *           incremented with a byte-width add and bytes 2 and 3 are cleared.
 *           Each of the last two stores reads the cell again, because the store
 *           before it may alias the cell. Nothing is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectWorkRecordClearFlag40ThenAdvanceWorkByte1AndClearWorkBytes2And3BmagicMagic15103_801EF2A0(
    void) {
  u8 *saved;
  u8 *record;
  u8 index;

  index = D_80146374;
  saved = g_battle_work;
  record = &D_80145E90[index * 0x140];
  g_battle_work = record;
  record[0] &= 0xBF;
  g_battle_work = saved;
  saved[1]++;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
}
