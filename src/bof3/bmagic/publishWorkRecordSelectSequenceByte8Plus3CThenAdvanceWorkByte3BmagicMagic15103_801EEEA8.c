#include "bof3/bof3.h"

/* Local work-record table in main RAM, outside this overlay's image. The
 * 0x140-byte-stride record array the front end and the battle overlays select
 * by a global byte; the exact sibling
 * selectWorkRecordClearFlag40ThenAdvanceWorkByte1AndClearWorkBytes2And3Bmagic
 * Magic15103_801EF2A0 publishes a record from the same table, and the asm
 * sibling func_801EEF24 right after this function selects it the same way.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added here. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */

/* Acting battler byte in main RAM, outside this overlay's image. The battle
 * targets own the address; this lift only reads it as the work-record index,
 * exactly as func_801EEF24 does when it publishes the record for the next
 * handler. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Scratchpad work-object pointer cell, already bound by this target's map row
 * g_battle_work = 0x1F800044. Declared as a named symbol rather than through
 * SPAD_PTR_SLOT so each access re-materialises the cell address with its own
 * lui, which is what the original does before the load and before the two
 * stores of the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Work-sequence selector of the shared EXE image, outside this overlay's
 * image. The ui lifts selectWorkSequenceForRoute7 and
 * selectWorkSequenceAndSetFactorIndex3 declare the same prototype and pass a
 * sequence id; Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, which the asm siblings func_801EF138 and
 * func_801EF484 also reference. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801EEEA8
 * @behavior Entry 0 of the work-byte-3 handler table at 0x801EEC24
 *           (0x801EEEA8, 0x801EEF24, 0x801EF388, 0x801EF484, 0x801EF2A0) and
 *           also entry 0 of the three-entry table at 0x801EEC10 that
 *           func_801EEE38 copies (0x801EEEA8, 0x801EEF24, 0x801EF070). It
 *           reads the acting battler byte D_80146374, saves the scratchpad
 *           work-object pointer cell 0x1F800044, and publishes the
 *           0x140-byte-stride local work record D_80145E90 + battler * 0x140
 *           at that cell. It selects the work sequence through func_8014D8D4
 *           with the new record's byte 8 plus 0x3C as the sequence id, then
 *           restores the previously published work object and advances that
 *           object's byte 3, the handler index of the work-byte-3 tables, by
 *           one. Nothing is returned; it takes no arguments and reads or
 *           writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkRecordSelectSequenceByte8Plus3CThenAdvanceWorkByte3BmagicMagic15103_801EEEA8(
    void) {
  u8 *saved;
  u8 *record;
  u8 index;

  index = D_80146374;
  saved = g_battle_work;
  record = &D_80145E90[index * 0x140];
  g_battle_work = record;
  func_8014D8D4((u8)(record[8] + 0x3C));
  g_battle_work = saved;
  saved[3]++;
}
