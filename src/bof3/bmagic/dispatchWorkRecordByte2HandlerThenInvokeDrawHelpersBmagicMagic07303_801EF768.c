#include "bof3/bof3.h"

typedef void (*BmagicMagic07303Byte2Handler)(void);

typedef struct BmagicMagic07303Byte2DispatchTable {
  BmagicMagic07303Byte2Handler handlers[3];
} BmagicMagic07303Byte2DispatchTable;

/*
 * Three-entry code pointer slice at 0x801EEC1C, the words this dispatcher
 * copies: entry 0 = 0x801EF864, entry 1 = 0x801EFA64, entry 2 = 0x801EFB14,
 * the overlay's three step handlers. The slice starts at word 6 of the
 * nine-word pointer run based at 0x801EEC04 (that run holds 0x801EF06C,
 * 0x801EF2DC, 0x801EF3D4, 0x801EF4E4, 0x801EF52C, 0x801EF6B8, 0x801EF864,
 * 0x801EFA64, 0x801EFB14 on disc), so this declaration covers only the three
 * words the scaled index adds to. The run sits in the overlay's leading data
 * block, outside the mapped main segment, so the reference binds through the
 * target's generated undefined-symbol table.
 */
extern BmagicMagic07303Byte2DispatchTable D_801EEC1C; /* @source 0x801EEC1C @kind data */

/*
 * Record pointer cell at 0x801F21F0, the last word of this payload. The
 * overlay frame loop at 0x801EEC28 publishes the current work record into this
 * cell with one `sw` at 0x801EECCC, so it holds a live record whenever this
 * dispatcher runs. Every access below re-reads the cell, so the original
 * re-materialises the cell address before each access instead of keeping the
 * record pointer live.
 */
extern u8 *D_801F21F0; /* @source 0x801F21F0 @kind data */

/* Shared per-frame counter in the main executable. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* File-local prototypes: the draw helpers are declared per caller. */
void func_801F00C4(void);
void drawFourRotatedSpokesFromPublishedCenterTintedByWorkByte5BmagicMagic07303_801EFBF8(
    s32 arg0, s32 arg1);
void drawFourRotatedTrianglesFromPublishedCenterWithHalfAndFullRadiusTintedByWorkByte5BmagicMagic07303_801EFE2C(
    s32 arg0, s32 arg1);

/* @source 0x801EF768
 * @behavior Dispatches one step of the current bmagic work record and then runs
 * that record's draw helpers. The record pointer is read from the cell at
 * 0x801F21F0 and the three code pointers at 0x801EEC1C (0x801EF864,
 * 0x801EFA64, 0x801EFB14) are copied into this function's own 0x28-byte frame;
 * the record unsigned byte at offset 2 selects one of those copies, which is
 * invoked with no arguments. After the handler returns the cell is re-read and,
 * when the record byte 0 has bit 0 set and its byte at offset 2 is non-zero,
 * the no-argument draw helper at 0x801F00C4 runs and then, when the record
 * unsigned halfword at 0xC has either low bit set, two draw helpers are called:
 * the spoke drawing step now named
 * drawFourRotatedSpokesFromPublishedCenterTintedByWorkByte5BmagicMagic07303_801EFBF8,
 * at 0x801EFBF8, with the low 16 bits of the shared frame counter at 0x80143E6C
 * halved and with the record byte at offset 6, and
 * drawFourRotatedTrianglesFromPublishedCenterWithHalfAndFullRadiusTintedByWorkByte5BmagicMagic07303_801EFE2C,
 * at 0x801EFE2C, with that halved
 * counter plus four and with the record byte at offset 6 added to half of
 * itself. It returns nothing, keeps the return address and the copied table in
 * the frame, and reads or writes no other state. No in-image caller names this
 * span, neither a `jal` nor a table word, so its entry belongs to the overlay
 * loader. The body was seeded from the exact 252-byte sibling at
 * emi/bmagic/magic168/03 0x801EF37C, from which it differs only in its three
 * relocated callee words and the record cell immediate, both re-read from this
 * payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkRecordByte2HandlerThenInvokeDrawHelpersBmagicMagic07303_801EF768(void) {
  BmagicMagic07303Byte2DispatchTable handlers;

  handlers = D_801EEC1C;
  handlers.handlers[D_801F21F0[2]]();
  if ((D_801F21F0[0] & 1u) && (D_801F21F0[2] != 0u)) {
    func_801F00C4();
    if (((u16 *)D_801F21F0)[6] & 3u) {
      drawFourRotatedSpokesFromPublishedCenterTintedByWorkByte5BmagicMagic07303_801EFBF8(
          (u16)(D_80143E6C >> 1), D_801F21F0[6]);
      drawFourRotatedTrianglesFromPublishedCenterWithHalfAndFullRadiusTintedByWorkByte5BmagicMagic07303_801EFE2C(
          (u16)((D_80143E6C >> 1) + 4),
          D_801F21F0[6] + (D_801F21F0[6] >> 1));
    }
  }
}
