#include "bof3/bof3.h"

typedef void (*BmagicMagic05803Byte2Handler)(void);

typedef struct BmagicMagic05803Byte2DispatchTable {
  BmagicMagic05803Byte2Handler handlers[3];
} BmagicMagic05803Byte2DispatchTable;

/*
 * Three-entry code pointer slice at 0x801EEC1C, the words this dispatcher
 * copies: entry 0 = 0x801EF454, entry 1 = 0x801EF5D4, entry 2 = 0x801EF61C.
 * They are the last three words of the nine-word pointer run the payload
 * header holds at 0x801EEC04 (0x801EED30, 0x801EEE78, 0x801EEEDC, 0x801EF008,
 * 0x801EF080, 0x801EF338, 0x801EF454, 0x801EF5D4, 0x801EF61C on disc). The run
 * fills the 36-byte tabledata segment that follows the four-byte header and
 * sits outside the mapped main segment, so the reference binds through this
 * target's generated undefined-symbol table and the declaration covers only
 * the three words the scaled index adds to.
 */
extern BmagicMagic05803Byte2DispatchTable D_801EEC1C; /* @source 0x801EEC1C @kind data */

/*
 * Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row. The original materialises the cell address once for the index read
 * before the handler call and once for the two byte reads that gate the tail,
 * so the cell is re-read after the call instead of being kept live across it.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * File-local prototypes: this overlay's two draw helpers. The first builds the
 * work object transform and pushes the matrix it works under; the second draws
 * inside that transform. Neither pops, so the pop stays in this dispatcher.
 */
void func_801EF69C(void);
void func_801EFA78(void);

/* @source 0x801EF3B0
 * @behavior Dispatches one frame of the overlay work handler from the scratchpad
 *           work object and then draws the work record. The work-object pointer
 *           cell at 0x1F800044 is read before frame creation; its unsigned byte
 *           at offset 2 selects an entry of the three-entry code pointer run at
 *           0x801EEC1C (0x801EF454, 0x801EF5D4, 0x801EF61C, the last three words
 *           of the nine-word run at 0x801EEC04), and that entry is invoked with
 *           no arguments. The three table words are copied into the frame of
 *           this function first, so the call target is read back from the copy
 *           and the selector is not bounds-checked. After the handler returns
 *           the cell is re-read and, when the work object byte 0 is non-zero
 *           and its byte at offset 2 is non-zero, the project helper at
 *           0x801EF69C runs, which pushes the matrix and installs the work
 *           object transform, then the draw helper at 0x801EFA78 draws under it,
 *           and PopMatrix restores the matrix. It returns nothing and keeps the
 *           return address at 0x20 across the dispatch; no other state is read
 *           or written. The second word of the two-word run at 0x801EFEEC holds
 *           this address (its first word holds 0x801EEF74), so a data table in
 *           this payload names the span and no jal reaches it; the first word of
 *           that run is the same dispatch and draw body gated on work byte 0
 *           alone and dispatching the 0x801EEC10 slice instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkBytes0And2SetBmagicMagic05803_801EF3B0(
    void) {
  BmagicMagic05803Byte2DispatchTable handlers;

  handlers = D_801EEC1C;
  handlers.handlers[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    func_801EF69C();
    func_801EFA78();
    PopMatrix();
  }
}
