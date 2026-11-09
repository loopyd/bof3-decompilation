#include "bof3/bof3.h"

typedef void (*BmagicMagic00603Byte2Handler)(void);

typedef struct BmagicMagic00603Byte2DispatchTable {
  BmagicMagic00603Byte2Handler handlers[3];
} BmagicMagic00603Byte2DispatchTable;

/*
 * Three-entry code pointer slice at 0x801EEC1C, the words this dispatcher
 * copies: entry 0 = 0x801F0100, entry 1 = 0x801F020C, entry 2 = 0x801F0290.
 * The slice is word 6 of the thirteen-word pointer run that starts at
 * 0x801EEC04 in the overlay's leading data block and runs on through
 * 0x801F05A4, 0x801F05F4, 0x801F0644 and 0x801F0684, so only the three words
 * the scaled index adds to are declared here. The run sits in a leading data
 * block that no map row owns, so the reference binds through this target's
 * generated undefined-symbol table, which already carries the row.
 */
extern BmagicMagic00603Byte2DispatchTable D_801EEC1C; /* @source 0x801EEC1C @kind data */

/*
 * Scratchpad work-object pointer cell at 0x1F800044, bound by its named cell.
 * The original builds the cell address twice, once for the index read before
 * the handler call and once for the two byte reads that gate the tail calls, so
 * the cell is read through the name instead of a live folded constant.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * Shared main-executable helper the overlay calls once the work object is
 * active. Unlifted and unnamed, so it keeps its raw name; this caller passes no
 * arguments and reads no result.
 */
void func_800AF764(void);

/*
 * This overlay's draw routine at 0x801F0308: copies the GTE matrix to the
 * scratchpad words at 0x1F800000, picks the draw-mode byte from GetGraphType
 * and emits one semi-transparent quad through the primitive cursor. Still an
 * asm row in this target, so it keeps its raw name; it takes no arguments here.
 */
void func_801F0308(void);

/* @source 0x801F0064
 * @behavior Copies the three words at 0x801EEC1C (0x801F0100, 0x801F020C,
 *   0x801F0290) into this function's own 0x28-byte frame and invokes the copy
 *   selected by the work object's unsigned byte at offset 2, read from the
 *   scratchpad pointer cell at 0x1F800044; the original does not bound the
 *   index. After the handler returns it re-reads the cell and, only when the
 *   work object's byte at offset 0 and its byte at offset 2 are both non-zero,
 *   calls the shared main-executable helper at 0x800AF764 and then this
 *   overlay's draw routine at 0x801F0308, in that order and with no arguments.
 *   The pointer load precedes frame creation as the original schedules it, and
 *   the frame holds $ra at 0x20 and the copied table at 0x10 through 0x18
 *   across the dispatch. Nothing is returned and no other state is read or
 *   written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeUpdateAndDrawWhenActiveBmagicMagic00603_801F0064(
    void) {
  BmagicMagic00603Byte2DispatchTable handlers;

  handlers = D_801EEC1C;
  handlers.handlers[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    func_800AF764();
    func_801F0308();
  }
}
