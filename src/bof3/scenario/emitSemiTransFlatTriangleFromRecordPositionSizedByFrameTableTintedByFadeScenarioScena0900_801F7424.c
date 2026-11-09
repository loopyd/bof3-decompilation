#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared engines this per-record emitter reaches: the grid-cell primitive
 * linker, the libgpu POLY_F3 setter verified in the shipped executable, and
 * the projector from a three-word point to a screen halfword pair. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A954(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);

/* The overlay-local four-halfword jitter table the record frame byte selects:
 * one 8-byte entry per frame value, four signed halfwords per entry. */
extern u16 D_801FE38C[];
extern u16 D_801FE38E[];
extern u16 D_801FE390[];
extern u16 D_801FE392[];

/* @source 0x801F7424
 * @behavior Per-record emitter of this overlay, the helper the active-record
 * sweep advanceAndRenderActiveRecordsInSelectedBankScenarioScena0900_801F75BC
 * calls for every record whose lead byte is set. It first arms the sprite draw
 * mode for the record: GetTPage(0, 1, 0x3C0, 0) yields the texture page, that
 * page is handed to SetDrawMode with the primitive published at g_PrimCursor
 * and both dither fields clear, and the 0x0C-byte DR_MODE packet is then linked
 * through the shared grid-cell primitive linker func_80155A08 at the record
 * position words 0x18 and 0x1C with kind 0 and size class 0x0C. The cursor slot
 * that follows is seeded as a POLY_F3 through func_8017A954, turned
 * semi-transparent, and the record position triple at 0x18 is projected into
 * that flat triangle's first corner by func_801AFF04. The second and third
 * corners are that projection plus the four signed halfwords of the 8-byte
 * jitter entry the record frame byte at 0x04 selects from the overlay tables at
 * 0x801FE38C, 0x801FE38E, 0x801FE390 and 0x801FE392, so x1 = x0 + halfword 0,
 * y1 = y0 + halfword 1, x2 = x0 + halfword 2 and y2 = y0 + halfword 3; the
 * triangle takes r0 = 0 with g0 and b0 both from the record fade byte at 0x03,
 * and its 0x14 bytes are linked as the same cell position with kind -2 and size
 * class 0x14. It takes the record base as its only argument, returns nothing and
 * writes no record field; the 0x58-byte frame holds the outgoing argument slot
 * of SetDrawMode, that review-flagged reservation and the saved registers of the
 * calls that outlive $v0.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape note: the record is reached only as the engine linker's two coordinate
 * words and the projector source, so the leading 0x28-byte record model the
 * caller keeps stays callers-local and this source reads the bytes it touches.
 * The frame byte at 0x04 and the fade byte at 0x03 are spelled at each site
 * rather than cached, because every halfword store through the primitive
 * aliases them and the original reloads both.
 *
 * Frame reservation: the original frame is 0x30 bytes larger than this body
 * owns - 0x58 against the 0x28 its own locals need - and every instruction
 * outside the prologue and epilogue already matches, so the reservation comes
 * from a declaration the original translation unit carried that emitted no code.
 * The bytes constrain only that size, so `reserved_frame` is a reconstruction
 * choice flagged for review and not recovered source: dropping it leaves an
 * otherwise exact 84-instruction body with a 0x28-byte frame, first difference
 * the prologue `addiu sp,sp,-0x28` at offset 0x0000.
 */
void emitSemiTransFlatTriangleFromRecordPositionSizedByFrameTableTintedByFadeScenarioScena0900_801F7424(
    u8* record) {
  POLY_F3* prim;
  u16      tpage;
  u8       reserved_frame[0x30];

  tpage = GetTPage(0, 1, 0x3C0, 0);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
  func_80155A08(FIELD_REF(s32, record, 0x18u), FIELD_REF(s32, record, 0x1Cu),
                0, 0xC);

  prim = (POLY_F3*)g_PrimCursor;
  func_8017A954(prim);
  SetSemiTrans(prim, 1);
  func_801AFF04(record + 0x18, &prim->x0);

  prim->x1 = prim->x0 + D_801FE38C[record[4] * 4];
  prim->y1 = prim->y0 + D_801FE38E[record[4] * 4];
  prim->x2 = prim->x0 + D_801FE390[record[4] * 4];
  prim->y2 = prim->y0 + D_801FE392[record[4] * 4];

  prim->r0 = 0;
  prim->g0 = record[3];
  prim->b0 = record[3];

  func_80155A08(FIELD_REF(s32, record, 0x18u), FIELD_REF(s32, record, 0x1Cu),
                -2, 0x14);
}
