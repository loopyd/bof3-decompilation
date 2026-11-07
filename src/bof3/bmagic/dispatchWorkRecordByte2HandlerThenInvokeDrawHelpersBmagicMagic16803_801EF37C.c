#include "bof3/bof3.h"

typedef void (*BmagicMagic16803Byte2Handler)(void);

typedef struct BmagicMagic16803Byte2DispatchTable {
  BmagicMagic16803Byte2Handler handlers[3];
} BmagicMagic16803Byte2DispatchTable;

/*
 * Three-entry code pointer slice at 0x801EEC1C, the words this dispatcher
 * copies: entry 0 = 0x801EF478, entry 1 = 0x801EF678, entry 2 = 0x801EF728,
 * the last three functions of the overlay's code block. The slice starts at
 * word 6 of the nine-word pointer run based at 0x801EEC04 (that run holds
 * 0x801EED38, 0x801EEFA0, 0x801EF048, 0x801EF158, 0x801EF1A0, 0x801EF2D8,
 * 0x801EF338, 0x801EF37C, 0x801EF478 on disc), so this declaration covers only
 * the three words the scaled index adds to. The run sits in the overlay's
 * leading data block, which no map row owns, so the reference binds through the
 * target's generated undefined-symbol table.
 */
extern BmagicMagic16803Byte2DispatchTable D_801EEC1C; /* @source 0x801EEC1C @kind data */

/*
 * Record pointer cell at 0x801F17B0. The overlay frame loop at 0x801EEC28
 * publishes the current work record into this cell with one `sw` at
 * 0x801EECEC, so it always holds a live record when this handler runs. Every
 * access below re-reads the cell, so the original re-materialises the cell
 * address before each access instead of keeping the record pointer live.
 */
extern u8 *D_801F17B0; /* @source 0x801F17B0 @kind data */

/* Shared per-frame counter in the main executable. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* File-local prototypes: the draw helpers are declared per caller. */
void func_801EFCD8(void);
void func_801EF80C(s32 arg0, s32 arg1);
void func_801EFA40(s32 arg0, s32 arg1);

/* @source 0x801EF37C
 * @behavior Dispatches one frame of this overlay's work record and then runs
 * the record's draw step. The current record pointer is read from the cell at
 * 0x801F17B0, the three words at 0x801EEC1C (0x801EF478, 0x801EF678,
 * 0x801EF728) are copied into this function's own 0x28-byte frame, and the
 * record's unsigned byte at offset 2 selects one of those copies, which is
 * invoked with no arguments. After the handler returns the cell is re-read and,
 * when the record's byte 0 has bit 0 set and its byte at offset 2 is non-zero,
 * the no-argument helper at 0x801EFCD8 runs and then, when the record's
 * unsigned halfword at 0xC has either low bit set, two draw helpers are called:
 * 0x801EF80C with the low 16 bits of the shared frame counter at 0x80143E6C
 * halved and with the record's byte at offset 6, and 0x801EFA40 with that
 * halved counter plus four and with the record byte at offset 6 added to half
 * of itself. It returns nothing, keeps $ra and the copied table in the frame,
 * and reads or writes no other state. The 252-byte span is byte-identical to
 * the still-unlifted 0x801EF37C of emi/bmagic/magic162/03, confirmed by a
 * payload compare, so that twin offers no source to clone; this body was seeded
 * from the original bytes and its shapes from the family dispatchers at
 * emi/bmagic/magic086/03 0x801EEC1C and emi/bmagic/magic00800/03 0x801F0568.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkRecordByte2HandlerThenInvokeDrawHelpersBmagicMagic16803_801EF37C(void) {
  BmagicMagic16803Byte2DispatchTable handlers;

  handlers = D_801EEC1C;
  handlers.handlers[D_801F17B0[2]]();
  if ((D_801F17B0[0] & 1u) && (D_801F17B0[2] != 0u)) {
    func_801EFCD8();
    if (((u16 *)D_801F17B0)[6] & 3u) {
      func_801EF80C((u16)(D_80143E6C >> 1), D_801F17B0[6]);
      func_801EFA40((u16)((D_80143E6C >> 1) + 4),
                    D_801F17B0[6] + (D_801F17B0[6] >> 1));
    }
  }
}
