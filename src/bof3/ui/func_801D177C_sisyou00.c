#include "bof3/ui/sisyou00_internal.h"

/* The original caller zero-extends both coordinates before this call, so this
 * translation unit declares the callsite-observed widths; the EMI-local
 * definition of func_801D23A0 keeps its own signed coordinates. */
void func_801D23A0(u16 arg0, u16 arg1, u8 arg2, u8 arg3);

/* @source 0x801D177C
 * @behavior draws one master-list record of the 164-byte record table at
 *           0x80144968 selected by the entry byte: the record icon kind
 *           through func_801D23A0, the record texture through func_8014F800,
 *           then three value rows formatted through sprintf into the shared
 *           D_80145AD4 buffer and emitted through func_80150098 with the
 *           vertical position taken from the record status bits; the
 *           highlight marker selected by the record status word is emitted
 *           through func_8014FC90, the record stream at 0x801D3F58 through
 *           func_801AF390, the record numeric row through func_801D1B28, and
 *           finally the draw mode, the panel primitive append and the icon
 *           primitive whose kind is the icon byte offset by three.
 * @status partial
 * @match 66.39
 * @residual 162/244 matched instructions and 976 current bytes versus 940 original; the 0x48 frame and every callee-saved assignment differ from the first instruction because the byte parameters stay in $fp instead of their stack slots
 */
void func_801D177C(u16 x, u16 y, u8 entry, u8 icon) {
  SisyouEntry* record;
  u8* base;
  s16 textX;
  s16 textY;
  u32 value;
  s32 align;

  base = &D_80144952;
  func_801AE3F0((u16)(x + 3), (u16)(y + 3), 0x7D, 0x30, 0, base[0]);
  record = (SisyouEntry*)(base + 0x16) + entry;
  func_801D23A0((u16)(x + 0x56), (u16)(y + 2), record->unk_05,
                (u8)((record->status.half >> 6) & 2));
  func_8014F800((s16)(x + 0x14), (s16)(y + 3), 0, 5, (u32)record);
  sprintf((char*)D_80145AD4, D_801D0C04, record->unk_06);
  func_80150098((s16)(x + 0x16), (s16)(y + 0x17), 0, D_80145AD4);

  if ((record->status.word & 0xA0) == 0xA0) {
    if (D_80143E6C & 0x20) {
      func_8014FC90((s16)(x + 0x2E), (s16)(y + 0x17), 1, 0x10, D_801D4048);
    } else {
      func_8014FC90((s16)(x + 0x2E), (s16)(y + 0x17), 1, 0x10, D_801D4050);
    }
  } else if (record->status.half & 0x80) {
    func_8014FC90((s16)(x + 0x2E), (s16)(y + 0x17), 1, 0x10, D_801D4048);
  } else if (record->status.half & 0x20) {
    func_8014FC90((s16)(x + 0x2E), (s16)(y + 0x17), 1, 0x10, D_801D4050);
  }

  value = record->unk_14;
  align = (record->status.half >> 11) & 4;
  if (value < 2U) {
    align = 2;
  }
  sprintf((char*)D_80145AD4, D_801D0C08, value);
  textX = (s16)(x + 0x16);
  textY = (s16)(y + 0x1F);
  func_80150098(textX, textY, align, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C10, record->unk_1C);
  func_80150098(textX, textY, 0, D_80145AD4);

  value = record->unk_16;
  align = (value == 0) ? 2 : (((record->unk_1E >> 2) >= value) ? 4 : 0);
  sprintf((char*)D_80145AD4, D_801D0C08, value);
  textY = (s16)(y + 0x27);
  func_80150098(textX, textY, align, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C10, record->unk_1E);
  func_80150098(textX, textY, 0, D_80145AD4);

  func_801AF390(x, y, D_801D3F58, 0);
  func_801D1B28((u16)(x + 0x14), (u16)(y + 0x12), entry, record->unk_06,
                record->unk_08);
  if (GetGraphType() != 1) {
    GetGraphType();
  }
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, 0xF, 0);
  func_8014E5A0(1, 0xC);
  emitPanelIconPrim((u16)(x + 9), (u16)(y + 5), (u8)(icon + 3), 0x1E,
                    (u16)GetClut(0x10, 0x1E0), 0x80);
}
