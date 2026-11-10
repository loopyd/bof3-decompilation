#include "bof3/bof3.h"

extern u8 D_80145AD4[];
extern u8 D_801EEC50[];
extern u8 D_801F0CD4[];

extern int sprintf(char* buffer, const char* format, ...);

/* Shared EXE helpers of the equipment record and icon path. func_80165E04
 * answers the palette byte the overlay table 0x801F0CD4 is indexed with;
 * func_80165D48 answers the twelve-byte name for the same type/index pair;
 * func_801AF20C answers the shading value the name is submitted with. Only
 * func_80165D48 is declared elsewhere in the tree (u8* func_80165D48(s32, u8)),
 * so the other two keep their address-anchored prototypes here. */
extern u32 func_80165E04(s32 arg0, u8 arg1);
extern u8* func_80165D48(s32 arg0, u8 arg1);
extern u32 func_801AF20C(const void* arg0);
extern void func_801AEA80(u16 arg0, u16 arg1, u8 arg2, s32 arg3);
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
extern void func_80150098(s16 arg0, s16 arg1, u32 arg2, const u8* arg3);

/* @source 0x801F0A74
 * @behavior Battle-end counted-list entry drawer: refuses to draw unless both
 *   the item index and the per-id count are non-zero, then draws one entry of
 *   the counted list. The palette byte the overlay table 0x801F0CD4 holds for
 *   the entry (indexed by func_80165E04 over the id split into its high byte
 *   and its low byte) is handed to func_801AEA80 together with the entry
 *   anchor, the anchor's row stepped down by two and the caller's last byte;
 *   the name func_80165D48 answers for the same split id is submitted at the
 *   anchor stepped right by ten through func_8014F800, with the id's high byte
 *   as the alignment and func_801AF20C of the name as the shading value. When
 *   the count is two or more it is formatted through the overlay "*%2d" at
 *   0x801EEC50 into the shared text buffer 0x80145AD4 and that buffer is
 *   submitted at the anchor stepped right by 0x6D on the row stepped down by
 *   two through func_80150098, once more with the id's high byte as the
 *   alignment. The 0x40-byte frame keeps $ra and $s0-$s7 across the six calls
 *   and carries no stack arguments of its own.
 * @status partial
 * @match 97.47
 * @residual The body is instruction-for-instruction equal to the original and
 *   the whole span is 0x13C bytes on both sides; the only miss is the emission
 *   order of two prologue save pairs, where the original stores $s4 (the entry
 *   copy of the zero-extended first argument) before $s7 (the saved
 *   argument 2) and this seed stores them the other way round. The same nine
 *   registers are saved and restored in the same order, so the residual class
 *   is register-save emission order, not allocation or expression shape.
 * 0x801F0A74 is the per-entry drawer that drawCountedListPanelBatlEnd00
 * (0x801F0650) calls once per non-zero id of the counted list, which passes
 * the id's high byte as the item type and its low byte as the item index. The
 * 13-byte palette table at 0x801F0CD4 holds 6, 8, 10, 9, 11, 12, 6, 6, 6, 6,
 * 20, 21, 22 and lives in the tail of the reviewed `textbin` row D_801F0C98,
 * one byte past the run start of the single unit glyph at 0x801F0CD0.
 */
void drawCountedListEntryBatlEnd00_801F0A74(s16 x, s16 y, s32 flags, u32 item_type,
                                            u8 item_index, u8 count, u8 arg6) {
  u32 cnt;
  u8* name;

  if (item_index == 0) {
    return;
  }
  cnt = count & 0xff;
  if (cnt == 0) {
    return;
  }
  func_801AEA80((u16)x, (u16)(y + 2),
                D_801F0CD4[func_80165E04(item_type & 0xff, item_index) & 0xff], arg6);
  name = func_80165D48(item_type & 0xff, item_index);
  func_8014F800((s16)(x + 0xa), (s16)y, flags & 0xff,
                func_801AF20C(name) & 0xff, (u32)name);
  if (cnt >= 2) {
    sprintf((char*)D_80145AD4, (const char*)D_801EEC50, cnt);
    func_80150098((s16)(x + 0x6d), (s16)(y + 2), flags & 0xff, D_80145AD4);
  }
}
