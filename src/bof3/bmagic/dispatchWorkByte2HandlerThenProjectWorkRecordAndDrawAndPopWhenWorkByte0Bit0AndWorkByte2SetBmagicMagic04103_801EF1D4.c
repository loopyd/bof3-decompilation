#include "bof3/bof3.h"

typedef void (*BmagicMagic04103Byte2Handler)(void);

typedef struct BmagicMagic04103Byte2DispatchTable {
  BmagicMagic04103Byte2Handler handlers[3];
} BmagicMagic04103Byte2DispatchTable;

/* Three-entry code pointer slice at 0x801EEC1C, the words this dispatcher
 * copies: entry 0 = 0x801EF27C, entry 1 = 0x801EF308, entry 2 = 0x801EF358, the
 * three work byte 2 stage handlers that follow the byte 1 helper run in the
 * overlay's leading data block. The run continues past these three words through
 * 0x801EEC28 and 0x801EEC2C, so this declaration only covers the prefix this
 * function copies. The block has no map row of its own, so the reference binds
 * through the target's generated undefined-symbol table. */
extern BmagicMagic04103Byte2DispatchTable D_801EEC1C; /* @source 0x801EEC1C @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The original builds the cell address once for the index
 * read before the handler call and once more for the two byte reads that gate
 * the tail, because the dispatched handler may republish the pointer, so the
 * cell is re-read after the call instead of being kept live across it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Pushes the GTE matrix stack and installs the work record's transform: the
 * record words at 0x34 and 0x38 (sra 9, biased by -0x4000) and signed halfword
 * 0x3E become the translation and rotation VECTOR, RotTrans projects it,
 * RotMatrix builds the record's matrix, MulMatrix2 composes it with the shared
 * screen matrix at 0x801492E8 and SetRotMatrix/SetTransMatrix install both GTE
 * matrices, leaving the pushed matrix for this function's PopMatrix. Still an
 * asm row in this target, so it keeps its raw name. */
void func_801EF5C0(void);

/* This overlay's own draw pass: it takes a draw mode from GetGraphType,
 * installs it with SetDrawMode, refreshes the scratchpad screen scale through
 * func_801782FC and func_801783C8 and emits the work-record effect with
 * SetSemiTrans, SetPolyG4 and RotTransPers4 under a second draw mode, with rand
 * and the record byte 0x9 picking the shade. Still an asm row in this target,
 * so it keeps its raw name; it draws under the matrix func_801EF5C0 installed
 * and does not pop it. */
void func_801EF670(void);

/* @source 0x801EF1D4
 * @behavior Dispatches one step of the overlay work handler: the work record is
 *   read from the scratchpad pointer cell at 0x1F800044 before frame creation,
 *   its unsigned byte at offset 0x2 is scaled by four and selects one entry of
 *   the three-entry code pointer slice at 0x801EEC1C, which is called with no
 *   arguments; the original does not bound the index. The three words are
 *   copied into this function's own 0x28-byte frame first, so the call target is
 *   read back from that copy. The cell is then re-read with its own lui/lw pair
 *   and the record's byte 0x0 has bit 0 tested, then the record's byte 0x2 is
 *   tested for non-zero, so only when both hold does the arm run: it installs
 *   the work record's transform with func_801EF5C0, draws the effect with
 *   func_801EF670 and then restores the GTE matrix stack with PopMatrix. The
 *   frame keeps the copied table at 0x10 and $ra at 0x20; the pointer-cell load
 *   is issued before frame creation. Performs no stores of its own, ignores the
 *   handler result, takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkByte0Bit0AndWorkByte2SetBmagicMagic04103_801EF1D4(
    void) {
  BmagicMagic04103Byte2DispatchTable handlers;

  handlers = D_801EEC1C;
  handlers.handlers[g_battle_work[2]]();
  if ((g_battle_work[0] & 1u) && (g_battle_work[2] != 0u)) {
    func_801EF5C0();
    func_801EF670();
    PopMatrix();
  }
}
