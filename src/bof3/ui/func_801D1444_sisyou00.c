#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D1444
 * @behavior draws the 164-byte record of the table at 0x80144968 selected by
 *           the entry byte as a master-summary panel at the (x, y) argument:
 *           the panel rectangle through func_801AE3F0, the four row-label
 *           textures "Pwr", "Def", "Int" and "Agl" through func_8014F800 with
 *           each row value read from the record and formatted through sprintf
 *           into the shared D_80145AD4 buffer and emitted through
 *           func_80150098, the selected badge texture through func_801502D0
 *           and func_8016620C once the record badge byte is not 0xFF, and
 *           finally the panel row streams at 0x801D3FE8, 0x801D4014 (once per
 *           each of the sixteen cells at x + cell*8) and 0x801D4020 through
 *           func_801AF390.
 * @status partial
 * @match 96.12
 * @residual 198/206 instructions and 820 current bytes versus 824 original; the residual hunks are the badge byte promotion (the original masks the loaded byte with andi v1,a0,0xFF before the 0xFF compare) and the panel-cell loop keeping a second live copy of x (the original copies x with move s2,s3 and uses s2 for the cell address and the final call). No measured clean-C spelling reproduces either one without adding an instruction, truncating the callee argument, or spilling the x parameter out of the 0x48 frame.
 */
void func_801D1444(s32 x, u16 y, u8 entry) {
  SisyouEntry* record;
  u8* base;
  u8* table;
  s32 offset;
  u8 i;

  base = &D_80144952;
  func_801AE3F0((u16)(x + 5), (u16)(y + 3), 0x94, 0x2D, 0, base[0]);

  /* The record table starts 0x16 bytes above the shared record base.  The
   * scaled byte offset is bound to a local first, so the table base keeps its
   * own register and the header offset folds into it in place instead of into
   * the scaled index. */
  offset = entry * sizeof(SisyouEntry);
  table = base + 0x16;
  record = (SisyouEntry*)(table + offset);

  func_8014F800((s16)(x + 0xB), (s16)(y + 8), 0, 0xFF, (u32)D_801D4038);
  sprintf((char*)D_80145AD4, D_801D0C04, record->unk_20);
  func_80150098((s16)(x + 0x30), (s16)(y + 0xA), 0, D_80145AD4);

  func_8014F800((s16)(x + 0xB), (s16)(y + 0x15), 0, 0xFF, (u32)D_801D403C);
  sprintf((char*)D_80145AD4, D_801D0C04, record->unk_22);
  func_80150098((s16)(x + 0x30), (s16)(y + 0x17), 0, D_80145AD4);

  func_8014F800((s16)(x + 0x53), (s16)(y + 8), 0, 0xFF, (u32)D_801D4040);
  sprintf((char*)D_80145AD4, D_801D0C04, record->unk_26);
  func_80150098((s16)(x + 0x78), (s16)(y + 0xA), 0, D_80145AD4);

  func_8014F800((s16)(x + 0x53), (s16)(y + 0x15), 0, 0xFF, (u32)D_801D4044);
  sprintf((char*)D_80145AD4, D_801D0C04, record->unk_24);
  func_80150098((s16)(x + 0x78), (s16)(y + 0x17), 0, D_80145AD4);

  if (record->unk_1B != 0xFF) {
    func_8016620C(0, 8, func_801502D0(record->unk_1B + 0x111));
    func_8014F800((s16)(x + 0xB), (s16)(y + 0x22), 0, 0xFF,
                  func_801502D0(0x34));
  }

  func_801AF390((s16)x, (s16)y, D_801D3FE8, 0);

  for (i = 0; i < 16; i++) {
    func_801AF390((s16)(x + i * 8), (s16)y, D_801D4014, 0);
  }

  func_801AF390((s16)x, (s16)y, D_801D4020, 0);
}
