#include "bof3/bof3.h"

typedef void (*BmagicMagic03903Byte2Handler)(void);

typedef struct BmagicMagic03903Byte2DispatchTable {
  BmagicMagic03903Byte2Handler handlers[6];
} BmagicMagic03903Byte2DispatchTable;

/*
 * Six-entry code pointer slice at 0x801EEC10, the words this dispatcher
 * copies: entry 0 = 0x801EF070, entry 1 = 0x801EF0D4, entry 2 = 0x801EF134,
 * entry 3 = 0x801EF1F4, entry 4 = 0x801EF320, entry 5 = 0x801EF44C, the six
 * work byte 2 stage handlers that follow the byte 1 run at 0x801EEC04 in the
 * overlay's leading data block. The run continues past these six words through
 * 0x801EEC28, 0x801EEC2C and 0x801EEC30, so this declaration only covers the
 * prefix this function copies. It sits in a block that no map row owns, so the
 * reference binds by the target's generated undefined-symbol table.
 */
extern BmagicMagic03903Byte2DispatchTable D_801EEC10; /* @source 0x801EEC10 @kind data */

/*
 * Scratchpad work-object pointer cell at 0x1F800044, bound by its named cell.
 * The original builds the cell address once for the index read before the
 * handler call and once for the two byte reads that gate the tail, so the cell
 * is re-read after the call instead of being kept live across it.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * File-local prototypes: this overlay's three draw helpers. None of them pops
 * the matrix it draws under, so the pop stays in this dispatcher.
 */
void func_801EF9F4(void);
void func_801EFAA4(void);
void func_801F0054(void);

/* @source 0x801EEFA8
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before frame creation, its unsigned byte at offset 2 selects an entry of the
 * six-entry code pointer run at 0x801EEC10 (0x801EF070, 0x801EF0D4,
 * 0x801EF134, 0x801EF1F4, 0x801EF320, 0x801EF44C), and that entry is invoked
 * with no arguments. The six table words are copied into this function's own
 * 0x30-byte frame first, so the call target is read back from the copy and the
 * selector is not bounds-checked. After the handler returns the cell is re-read
 * and, when the work object's byte 0 has bit 0 set and its byte at offset 2 is
 * non-zero, this overlay's draw helpers at 0x801EF9F4, 0x801EFAA4 and
 * 0x801F0054 run in that order and PopMatrix then restores the matrix the
 * first of them pushed. It returns nothing and keeps $ra at 0x28 across the
 * dispatch; no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeDrawHelpersAndPopWhenActiveBmagicMagic03903_801EEFA8(
    void) {
  BmagicMagic03903Byte2DispatchTable handlers;

  handlers = D_801EEC10;
  handlers.handlers[g_battle_work[2]]();
  if ((g_battle_work[0] & 1u) && (g_battle_work[2] != 0u)) {
    func_801EF9F4();
    func_801EFAA4();
    func_801F0054();
    PopMatrix();
  }
}
