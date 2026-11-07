#include "bof3/bof3.h"

/* 0x140-byte front-end record rows of the shared main-executable data
 * segment. Only the stride is proven here: the original divides the raw byte
 * distance between the published record pointer and the table base by
 * exactly 0x140, and the plchar accessors reach byte 0x12B of the record
 * published at 0x80146250, which stays inside that stride. */
typedef struct World02Area11113FrontEndRecord {
  u8 bytes[0x140];
} World02Area11113FrontEndRecord;

/* Shared scroll pair of the exe-resident world state: the halfword at
 * 0x80145EC6 and the halfword four bytes later at 0x80145ECA, both read here
 * with `lh` so they are signed halfwords. */
extern s16 D_80145EC6[]; /* @source 0x80145EC6 @kind data */
extern s16 D_80145ECA; /* @source 0x80145ECA @kind data */

/* Packed cell-code map inside this overlay's trailing data blob: two nibbles
 * per byte, four bytes per map column, so the original addresses it as
 * column * 4 + row / 2. */
extern const u8 D_801F5C18[]; /* @source 0x801F5C18 @kind data */

/* Base of the shared 0x140-byte front-end record table and the pointer cell
 * that publishes the record the front end currently works on. */
extern World02Area11113FrontEndRecord D_80145E90[]; /* @source 0x80145E90 @kind table */
extern World02Area11113FrontEndRecord* D_80146250; /* @source 0x80146250 @kind unknown */

/* Record published through the global pointer cell at 0x80148210; this body
 * saves and restores its halfword at +0x0A around the request call. */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* Selector byte of the 0x98-byte record cells the scroll-cell selection
 * writes: 0x80147A60 is byte 8 of the table based at 0x80147A58, addressed by
 * the original as its own absolute symbol. */
extern u8 D_80147A60[]; /* @source 0x80147A60 @kind table */

/* Shared byte that mirrors the selector code published to the work record. */
extern u8 D_80143F50; /* @source 0x80143F50 @kind unknown */

/* Shared front-end request helper: takes the work record, a work-mode byte,
 * the selected cell slot, a one-byte argument word and a sub-command. */
void func_801AC768(u16* record, s32 mode, u8 slot, u8 argument, s32 command);

/* @source 0x801F35F4
 * @behavior Resolves the same packed-map cell the selector step at 0x801F4510
 *           uses: it halves the signed halfword at 0x80145ECA minus 0xB1 and
 *           the signed halfword at 0x80145EC6 minus 0xB, reads the nibble at
 *           0x801F5C18[col * 4 + ((row & 0xFF) >> 1)] (low nibble on an odd
 *           row, high nibble on an even one) and takes that nibble minus one
 *           as the record slot, 0xFF meaning no selection. While a slot is
 *           selected it requests work mode 7 through the shared helper at
 *           0x801AC768 with the cell slot and the one-based ordinal of the
 *           front-end record published at 0x80146250 inside the 0x140-byte
 *           table at 0x80145E90, sub-command 0x10, and the record halfword at
 *           +0x0A is saved before the call and written back afterwards. It
 *           then publishes that cell's stored selector byte (the byte at
 *           0x80147A60 + slot * 0x98) masked with 0x7 to byte 8 of the work
 *           record behind the scratchpad pointer cell 0x1F800044 and to the
 *           shared byte at 0x80143F50. Takes no arguments and returns
 *           nothing; the 0x28-byte frame keeps $ra, $s0 and $s1.
 *
 * Name evidence: 0x80145EC6/0x80145ECA are the shared scroll pair and
 *           0x801F5C18 the packed cell map the selector step in this payload
 *           already names, the 0x98-byte offset computes byte 8 of the same
 *           record cells the selector step writes, 0x80143F50 is the shared
 *           mirror that byte feeds, and 0x80145E90 is the 0x140-byte record
 *           table the shared plchar accessors index while 0x80146250
 *           publishes the record they work on. The work-mode byte the request
 *           seeds and the meaning of sub-command 0x10 belong to the resident
 *           helper, so the name stays at the request.
 *
 * Shape: no byte-identical twin exists. The near neighbour at 0x801F4510
 *           shares the first forty instructions of the lookup (same `lh` pair,
 *           same subtract-then-halve forms, same odd-row arm out of line behind
 *           `j`) but ends on a different tail, and the inline copy tucked into
 *           0x801F39E0 carries mode 0xC and a different epilogue, so this body
 *           is seeded from the original bytes. The ordinal division is the
 *           compiler's exact pointer-quotient form: because the operands are
 *           row pointers, the quotient is the exact division the original
 *           emits, not a general signed divide.
 * @status partial
 * @match 86.02
 * @residual 80/93 instructions, 372 of 372 bytes; first difference at
 *           0x801F3640 (index 19). A byte row quotient with branch-local
 *           unsigned halving reproduces the row copy, both row masks and
 *           shifts, branch targets and body size. The column mask destination
 *           and the base/column address scheduling still differ in both arms.
 *           The request and publication tail remains byte-identical.
 */
void publishSelectedWorkRecordCodeFromScrollCellAndRequestFrontEndOrdinalWorld02Area11113_801F35F4(
    void) {
  const u8* cell;
  u8 col;
  u8 row;
  u8 code;
  u8 slot;
  u16 saved;
  u8 ordinal;

  col = (D_80145ECA - 0xB1) / 2;
  row = (D_80145EC6[0] - 0xB) / 2;
  if (row & 1) {
    cell = (const u8*)D_801F5C18 + (col & 0xFF) * 4;
    code = cell[row >> 1] & 0xF;
  } else {
    cell = (const u8*)D_801F5C18 + (col & 0xFF) * 4;
    code = cell[row >> 1] >> 4;
  }
  slot = code - 1;
  if (slot != 0xFF) {
    ordinal = (u8)(D_80146250 - D_80145E90 + 1);
    saved = D_80148210[5];
    func_801AC768(D_80148210, 7, slot, ordinal, 0x10);
    D_80148210[5] = saved;
    code = D_80147A60[slot * 0x98] & 0x7;
    SPAD_PTR_SLOT(u8, 0x44)[8] = code;
    D_80143F50 = code;
  }
}
