#include "bof3/bof3.h"

typedef void (*BmagicMagic16203Handler)(void);

typedef struct BmagicMagic16203DispatchTable {
  BmagicMagic16203Handler handlers[6];
} BmagicMagic16203DispatchTable;

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The original loads it at the head of the body, before the frame is created,
 * and reads its unsigned byte 0x01 for the table index. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* The overlay's battle-magic record pool at 0x801F01B0: 128 records of stride
 * 0x2C, the pool reserveFirstFreeRecord hands out.
 * Byte 0 bit 0 is the in-use flag this sweep tests, and the word at record
 * offset 0x28 (0x801F01D8 for the first record) is the work pointer installed
 * into 0x801EB4E0 for the record's step. The flags read re-materialises this
 * base, while the published record pointer reuses one materialised base held
 * in a saved register, so the original keeps both forms. */
extern u8 magic16203RecordFlags[]; /* @source 0x801F01B0 @kind data */

/* Current-record pointer cell at 0x801F17B0, the tail word of the overlay's
 * trailing data block. The sweep publishes the record it is about to step
 * here; stateClearRecordHead and the record
 * handlers read it back. No map row names the cell, so the reference binds
 * through the target's generated undefined-symbol table. */
extern u8 *D_801F17B0; /* @source 0x801F17B0 @kind data */

/* Shared work pointer cell at 0x801EB4E0, outside this overlay image. The
 * sweep saves the cell, installs the record's own work pointer and restores it
 * once the record's step handler returns. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* The overlay header's nine-word code-pointer run at 0x801EEC04: this payload's
 * 0x801EED38, 0x801EEFA0, 0x801EF048, 0x801EF158, 0x801EF1A0, 0x801EF2D8,
 * 0x801EF478, 0x801EF678 and 0x801EF728, the overlay's own byte-1 step bodies.
 * This body copies the first six words into its frame, so the declaration
 * covers that prefix and not the run. The run sits in the leading `tabledata`
 * block, which no map row labels, so the reference binds through this target's
 * generated undefined-symbol table. */
extern BmagicMagic16203DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* This target's record byte-1 step dispatcher at 0x801EF338, exact in
 * src/bof3/bmagic/dispatchRecordByte.c: it
 * reads the record published at 0x801F17B0 and runs the handler that record's
 * unsigned byte 1 selects, so publishing a record there makes it run that
 * record's step. */
extern void dispatchRecordByte(void); /* @source 0x801EF338 @kind function */

/* @source 0x801EEC28
 * @behavior Per-frame entry of this overlay. It copies the six code-pointer
 *           words at 0x801EEC04 - the first six entries of the overlay header's
 *           nine-word handler run, this payload's 0x801EED38, 0x801EEFA0,
 *           0x801EF048, 0x801EF158, 0x801EF1A0 and 0x801EF2D8 - into its own
 *           0x38-byte frame at 0x10 through 0x24 and calls the copy's entry
 *           selected by the unsigned byte 0x01 of the work object published at
 *           0x1F800044, with no arguments and no bound on the index; the cell is
 *           read before the frame is created and the selected word is read back
 *           from the frame copy. It then walks the 128 records of the
 *           0x2C-stride pool at 0x801F01B0 and, for each record whose flags byte
 *           has bit 0 set, saves 0x801EB4E0, loads the record's word 0x28 - the
 *           word at 0x801F01D8 for that index - publishes the record itself into
 *           the current-record cell at 0x801F17B0, stores that word into
 *           0x801EB4E0, runs the record's byte-1 step through the dispatcher at
 *           0x801EF338 and restores 0x801EB4E0. The flags read re-materialises
 *           the pool base, while the published record pointer reuses one
 *           materialised base held in a saved register, so the original keeps
 *           both forms. Takes no arguments and returns nothing; the frame only
 *           keeps $ra and $s0 to $s2 across the two calls. No jal and no payload
 *           word names 0x801EEC28, so the overlay loader is its only caller and
 *           the parameter roles stay structural. The 0x110-byte body is
 *           byte-identical to emi/bmagic/magic168/03 0x801EEC28 at the same
 *           payload offset, though that span was still on its asm row when this
 *           lift was made. The statement order is the one the exact lift of the
 *           same-shape entry emi/bmagic/magic021/03 0x801EEC28 uses, which
 *           publishes into the scratchpad work cell instead of the record cell;
 *           the pool sweep follows this target's own exact
 *           reserveFirstFreeRecord for the stride,
 *           slot count and mask arithmetic.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkByte1Records(void) {
  BmagicMagic16203DispatchTable handlers;
  u8 index;
  u32 offset;
  u8 *record;
  u8 *savedRecord;
  u8 *recordWork;

  handlers = D_801EEC04;
  handlers.handlers[g_battle_work[1]]();
  for (index = 0; index < 0x80; index++) {
    offset = index * 0x2C;
    if (magic16203RecordFlags[offset] & 1) {
      record = &magic16203RecordFlags[offset];
      savedRecord = D_801EB4E0;
      recordWork = *(u8 **)&magic16203RecordFlags[offset + 0x28];
      D_801F17B0 = record;
      D_801EB4E0 = recordWork;
      dispatchRecordByte();
      D_801EB4E0 = savedRecord;
    }
  }
}
