#include "bof3/bof3.h"

typedef void (*BmagicMagic22303Handler)(void);

typedef struct BmagicMagic22303DispatchTable {
  BmagicMagic22303Handler handlers[3];
} BmagicMagic22303DispatchTable;

/* Scratchpad work-object pointer cell bound as g_battle_work by this target's
 * map. The original reads it at the head of the body, before the frame is
 * created, for the byte-1 dispatch, and reads it again inside the sweep to save
 * it across the record step. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Record pool of this overlay at 0x801EFC60: 64 records of stride 0x78, the
 * same pool reserveFirstFreeRecordBmagicMagic22303_801EFB78 claims from. Byte 0
 * bit 0 is the in-use flag this sweep tests, and the word at 0x74 of a record
 * holds the work pointer stored when the record was claimed. The flags read
 * re-materialises this base, so the array keeps the pool name. */
extern u8 D_801EFC60[]; /* @source 0x801EFC60 @kind data */

/* Shared battle record pointer at 0x801EB4E0 outside this overlay, named by the
 * generated undefined_syms_auto.txt row. The sweep saves the cell, publishes the
 * record's own work pointer into it and restores it once the record's step
 * handler returns. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* The overlay header's three-word code-pointer run at 0x801EEC04 holds this
 * payload's words 0x801EED18, 0x801EEF4C and 0x801EEFB4, the overlay's own
 * byte-1 step bodies. The run sits in the leading `tabledata` block, which no
 * map row labels, so the reference binds through this target's generated
 * undefined-symbol table, and the body copies the whole run into its frame. */
extern BmagicMagic22303DispatchTable
    D_801EEC04; /* @source 0x801EEC04 @kind data */

/* This overlay's record-step dispatcher at 0x801EF3C0, exact through
 * src/bof3/bmagic/magic22303DispatchTables.c: it reads the record published at
 * 0x1F800044 and calls the handler its unsigned byte 1 selects from the
 * four-word run at 0x801EFC40, so publishing a record there runs that record's
 * step. */
extern void callStateTablefc40(
    void); /* @source 0x801EF3C0 @kind function */

/* @source 0x801EEC10
 * @behavior Per-frame entry of this overlay. It copies the three code-pointer
 *           words at 0x801EEC04 - this payload's 0x801EED18, 0x801EEF4C and
 *           0x801EEFB4 - into its own 0x38-byte frame at 0x10 through 0x18 and
 *           calls the copy's entry selected by the unsigned byte 0x01 of the
 *           work object published at 0x1F800044, with no bound on the index; the
 *           cell is read before the frame is created and the selected word is
 *           read back from the frame copy. It then walks the 64 records of the
 *           0x78-stride pool at 0x801EFC60 and, for each record whose flags byte
 *           has bit 0 set, saves the scratchpad work cell and 0x801EB4E0, loads
 *           the record's word 0x74 - the word at 0x801EFCD4 for that index -
 *           publishes the record itself as the work object and that word into
 *           0x801EB4E0, runs that record's step through the dispatcher at
 *           0x801EF3C0 and restores both cells. The flags read re-materialises
 *           the pool base, while the published record pointer reuses one
 *           materialised base held in a saved register, so the original keeps
 *           both forms. Takes no arguments and returns nothing; the frame keeps
 *           $ra and $s0 to $s3 across the two calls. No jal and no payload word
 *           names 0x801EEC10, so the overlay loader is its only caller and the
 *           parameter roles stay structural. A payload cmp proved bytes
 *           0x801EEC10..0x801EED17 equal to the 0x108-byte span at 0x801EEC28 of
 *           emi/bmagic/magic021/03 apart from the three words that carry its own
 *           pool base (0x1B4 higher there) and the word that carries its own
 *           step-dispatcher jal target, so the span has no byte-identical twin
 *           and statement order, operand forms, offset-local spelling and
 *           declaration style follow that exact lift with this function's own
 *           pool base, record count, header prefix and callee.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Tableec04ThenPublishEachOccupiedRecordAndRunItsStepBmagicMagic22303_801EEC10(
    void) {
  BmagicMagic22303DispatchTable handlers;
  u8 index;
  u32 offset;
  u8 *record;
  u8 *savedWork;
  u8 *savedRecord;
  u8 *recordWork;

  handlers = D_801EEC04;
  handlers.handlers[g_battle_work[1]]();
  for (index = 0; index < 0x40; index++) {
    offset = index * 0x78;
    if (D_801EFC60[offset] & 1) {
      record = &D_801EFC60[offset];
      savedWork = g_battle_work;
      savedRecord = D_801EB4E0;
      recordWork = *(u8 **)&D_801EFC60[offset + 0x74];
      g_battle_work = record;
      D_801EB4E0 = recordWork;
      callStateTablefc40();
      D_801EB4E0 = savedRecord;
      g_battle_work = savedWork;
    }
  }
}
