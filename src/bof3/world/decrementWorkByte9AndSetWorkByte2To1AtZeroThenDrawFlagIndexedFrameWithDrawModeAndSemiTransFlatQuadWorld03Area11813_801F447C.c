#include "bof3/bof3.h"

extern u8 D_80144ED0; /* @source 0x80144ED0 @kind unknown */
extern u8 D_801F5138[];
extern SVECTOR D_1F800014[]; /* @source 0x1F800014 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind unknown */

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A9A4(void* arg0);
s32 GetGraphType(void);
void drawRecordCellTexturedQuadWorld03Area11813_801F4634(s32 arg0, s32 arg1);
void func_801F48E0(void);

/* @source 0x801F447C
 * @behavior Byte-2 state handler that the last byte-2 dispatch table of this
 *           overlay (0x801F519C, indexed by the dispatcher at 0x801F410C) lists
 *           at entry 4, the arm the handler at 0x801F43FC reaches by advancing
 *           the record state byte. Decrements the work record byte at offset
 *           0x09 and, once that countdown reads zero, stores 1 into the record
 *           state byte at offset 0x02 so the next dispatch restarts the cycle
 *           at entry 1. Takes the draw-mode texture page from GetGraphType,
 *           0x2A5 for graphics type 1 or 2 and 0xB5 otherwise, seeds that
 *           draw mode on the shared primitive cursor, and links the 0xC-byte
 *           draw-mode packet at the record position pair 0x34/0x38 through the
 *           shared grid-cell linker func_80155A08. Draws the record through the
 *           overlay record draw routine 0x801F4634 with frame index record byte
 *           0x0B plus four times the probe of the flag number that byte indexes
 *           in table 0x801F5138, taken through the shared flag tester
 *           func_8015B5D4 against the flag bank at 0x80144ED0, at size 0x40. It
 *           then re-seeds the same cursor as a flat 0x18-byte packet through
 *           func_8017A9A4, sets its semi-transparency bit, writes record byte
 *           0x09 times 30 into its red, green and blue bytes, projects the four
 *           scratchpad vectors at 0x1F800014 into its four screen pairs with
 *           RotTransPers4, links that packet at the same position pair and
 *           finishes with the overlay primitive setup 0x801F48E0. Takes no
 *           arguments and returns nothing; the record pointer is loaded before
 *           the 0x38-byte frame is created, that frame keeps only $ra at 0x34
 *           and $s0 at 0x30, and the record cell is reloaded for each use.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndSetWorkByte2To1AtZeroThenDrawFlagIndexedFrameWithDrawModeAndSemiTransFlatQuadWorld03Area11813_801F447C(
    void) {
  POLY_F4* prim;
  long depth;
  long flag;
  s32 mode;

  D_1F800044[9]--;
  if (D_1F800044[9] == 0) {
    D_1F800044[2] = 1;
  }
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    mode = 0x2A5;
  } else {
    mode = 0xB5;
  }
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, mode, 0);
  func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                FIELD_REF(s32, D_1F800044, 0x38), 1, 0xC);
  drawRecordCellTexturedQuadWorld03Area11813_801F4634(
      D_1F800044[0xB] +
          (func_8015B5D4((u32)&D_80144ED0, D_801F5138[D_1F800044[0xB]]) << 2) +
          1,
      0x40);

  prim = (POLY_F4*)g_PrimCursor;
  func_8017A9A4(prim);
  SetSemiTrans(prim, 1);
  prim->r0 = prim->g0 = prim->b0 = D_1F800044[9] * 30;
  RotTransPers4(D_1F800014, D_1F800014 + 1, D_1F800014 + 2, D_1F800014 + 3,
                (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                (long*)&prim->x3, &depth, &flag);
  func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                FIELD_REF(s32, D_1F800044, 0x38), 1, 0x18);
  func_801F48E0();
}
