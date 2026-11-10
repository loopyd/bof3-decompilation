#include "bof3/bof3.h"

typedef void (*BmagicMagic04103Byte2HandlerFour)(void);

typedef struct BmagicMagic04103Byte2DispatchTableFour {
  BmagicMagic04103Byte2HandlerFour handlers[4];
} BmagicMagic04103Byte2DispatchTableFour;

/* Four-entry code pointer slice at 0x801EEC28, the words this dispatcher
 * copies: entry 0 = 0x801EF484, entry 1 = 0x801EF4D4, entry 2 = 0x801EF524,
 * entry 3 = 0x801EF564, the four work byte 2 stage handlers that follow the
 * byte 1 helper run in the overlay's leading data block. This slice is the tail
 * of that block, which ends at 0x801EEC38; the three words from 0x801EEC1C that
 * the sibling dispatcher at 0x801EF1D4 copies are its head, so this declaration
 * covers only the four words this function copies. The block has no map row of
 * its own, so the reference binds through the target's generated
 * undefined-symbol table. */
extern BmagicMagic04103Byte2DispatchTableFour D_801EEC28; /* @source 0x801EEC28 @kind data */

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

/* This overlay's own triangle draw pass: it takes a draw mode from GetGraphType,
 * installs it with SetDrawMode, refreshes the scratchpad screen scale through
 * func_801782FC and func_801783C8 and emits the work-record effect with
 * SetSemiTrans and RotTransPers3 under a second draw mode, with func_8014E5A0
 * and func_8017A97C around it. Still an asm row in this target, so it keeps its
 * raw name; it draws under the matrix func_801EF5C0 installed and does not pop
 * it. */
void func_801EFAB8(void);

/* @source 0x801EF3D0
 * @behavior Dispatches one step of the overlay work handler: the work record is
 *   read from the scratchpad pointer cell at 0x1F800044 before frame creation,
 *   its unsigned byte at offset 0x2 is scaled by four and selects one entry of
 *   the four-entry code pointer slice at 0x801EEC28, which is called with no
 *   arguments; the original does not bound the index. The four words are copied
 *   into this function's own 0x28-byte frame first, so the call target is read
 *   back from that copy. The cell is then re-read with its own lui/lw pair and
 *   the record's byte 0x0 has bit 0 tested, then the record's byte 0x2 is tested
 *   for non-zero, so only when both hold does the arm run: it installs the work
 *   record's transform with func_801EF5C0, draws the effect's triangles with
 *   func_801EFAB8 and then restores the GTE matrix stack with PopMatrix. The
 *   frame keeps the copied table at 0x10 and $ra at 0x20; the pointer-cell load
 *   is issued before frame creation. Performs no stores of its own, ignores the
 *   handler result, takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2FourEntryHandlerThenProjectWorkRecordAndDrawTrianglesAndPopWhenWorkByte0Bit0AndWorkByte2SetBmagicMagic04103_801EF3D0(
    void) {
  BmagicMagic04103Byte2DispatchTableFour handlers;

  handlers = D_801EEC28;
  handlers.handlers[g_battle_work[2]]();
  if ((g_battle_work[0] & 1u) && (g_battle_work[2] != 0u)) {
    func_801EF5C0();
    func_801EFAB8();
    PopMatrix();
  }
}
