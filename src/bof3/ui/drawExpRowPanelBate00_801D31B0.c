#include "bof3/bof3.h"

/* Shared party-record table of the main executable. This body reads its byte 0
 * as the CLUT-bank argument of the inset painter and forms the selected record
 * 0x16 bytes past that byte, so the leading byte is the only cell named here.
 * @source 0x80144952 @kind data */
extern u8 D_80144952;

/* Shared sprintf work buffer of the main executable, outside this EMI payload:
 * the formatted value row is written here and then handed to the line-text
 * drawer as the buffer address.
 * @source 0x80145AD4 @kind data */
extern u8 D_80145AD4[];

/* Decimal format and eight-dash placeholder of this overlay's leading data
 * blob, at payload bytes 0x34 and 0x38: the value row uses the first, and the
 * second replaces it when the value reads -1.
 * @source 0x801D0C34 @kind data */
extern const char D_801D0C34[];
/* @source 0x801D0C38 @kind data */
extern const char D_801D0C38[];

/* Caption strings of this overlay's trailing data blob, eight and twelve bytes
 * past the six-byte equip id table: the mode argument selects between them.
 * @source 0x801D8B48 @kind data */
extern const char D_801D8B48[];
/* @source 0x801D8B4C @kind data */
extern const char D_801D8B4C[];

/* Sprite-record stream of this overlay's trailing data blob, submitted at the
 * panel origin with the flag byte 1.
 * @source 0x801D8984 @kind data */
extern const u8 D_801D8984[];

/* Inset rectangle painter of the main executable: the two coordinates, the
 * width, the height, the shade and the CLUT-bank byte. Both coordinates arrive
 * zero-extended here, so those are the call-site widths.
 * @source 0x801AE3F0 */
extern void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u8 arg5);

/* Caption drawer of the main executable: the two signed coordinates, the
 * alignment argument 0, the 0xFF mask and the caption address.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Line-text drawer of the main executable: the two signed coordinates, the
 * shade and the buffer it draws.
 * @source 0x8014FF0C */
extern void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

extern int sprintf(char* buffer, const char* format, ...);

/* Experience-table sum of this overlay, defined by sumLevelExpBate00_801D4FD0.c:
 * the character row and the number of level entries it adds. */
extern s32 sumLevelExpBate00_801D4FD0(u8 row, u8 count);

typedef struct PartyRecord {
  u8 unk_00[6];
  u8 level;   /* +0x06 level byte the experience sum walks past */
  u8 unk_07;  /* +0x07 */
  u32 exp;    /* +0x08 experience word the mode-zero path shows */
  u8 unk_0C[0x98];
} PartyRecord;

/* @source 0x801D31B0
 * @behavior Experience panel of the BATE overlay: it fills the 0x65 by 0x25
 *           inset rectangle at (x+3, y+3) through func_801AE3F0 with shade 0
 *           and the CLUT-bank byte D_80144952[0], then selects the 164-byte
 *           party record D_80144952[index*164+0x16]. With the mode byte zero it
 *           draws the EXP caption D_801D8B48 through func_8014F800 at
 *           (x+0x2C, y+7) with the mask 0xFF and takes the word at record+8 as
 *           its value; with the mode byte set it draws the NEXT caption
 *           D_801D8B4C at (x+0x26, y+7) with the same mask and takes as its
 *           value the sum sumLevelExpBate00_801D4FD0 forms over the first
 *           record[6]+1 level entries of that character row. It formats the
 *           value into the shared buffer D_80145AD4 with the format D_801D0C34
 *           when it is not -1 and with the placeholder D_801D0C38 when it is,
 *           draws that buffer through func_8014FF0C at (x+0x15, y+0x1A) with
 *           shade 0 and finally submits the sprite stream D_801D8984 at (x, y)
 *           through func_801AF390 with flag 1. Nothing is returned and the
 *           0x50-byte frame keeps $s0 to $s5 and $ra across the calls. No jal
 *           or table word of this payload names the span, so its four arguments
 *           have the roles their uses give them: a caller outside the payload
 *           invokes it. The 0x1D4 bytes equal under cmp the emi/etc/shop/00
 *           span at 0x801D8C58 apart from ten words: the five jump and jal
 *           targets of the body, each 0x5AA8 above the shop copy's own, and the
 *           five literal low halves - the three blob cells at +0xCC, +0xDC and
 *           +0x1A0 each 0x3304 above the shop copy's cells and the two
 *           format-string low halves at +0x138 and +0x158 each 0xC4 below them,
 *           while their lui halves stay equal - so this is one body with this
 *           overlay's own bindings, and the shop copy is shape evidence only.
 * @status partial
 * @match 65.81
 * @residual the record pointer keeps a second callee-saved register: the
 *           measured source saves $s0 through $s6 where the original saves $s0
 *           through $s5 and reuses the base register for the record, so this
 *           body spends one extra save and one extra restore and carries one
 *           instruction fewer than the original.
 *
 * Matching note: the frame is 0x50 bytes and 0x18 of them are read and written
 * by no emitted instruction, so they are the size-only reservation
 * `frame_reserve` below; only its size is constrained and its name and type are
 * a reconstruction choice. Shapes measured and reverted: the record formed as
 * `base + 0x16 + index * 164` (448 of 468 bytes), as `index * 164 + (base +
 * 0x16)` (448), as a walk of the record table symbol D_80144968 (448), in
 * place on one pointer (448), and stepped from the base pointer without the
 * byte symbol (448). The retained shape measures 456 of 468 bytes with the
 * prologue register order, the caption select and the tail equal to the
 * original.
 */
void drawExpRowPanelBate00_801D31B0(s16 x, s16 y, u8 index, u8 mode) {
  u8* base;
  PartyRecord* record;
  s32 value;
  u8 frame_reserve[0x18]; /* size-only frame reservation, see the note above */

  base = &D_80144952;
  func_801AE3F0((u16)(x + 3), (u16)(y + 3), 0x65, 0x25, 0, base[0]);
  record = (PartyRecord*)(&D_80144952 + 0x16) + index;
  func_8014F800((s16)(x + (mode == 0 ? 0x2C : 0x26)), (s16)(y + 7), 0, 0xFF,
                (u32)(mode == 0 ? D_801D8B48 : D_801D8B4C));
  if (mode == 0) {
    value = record->exp;
  } else {
    value = sumLevelExpBate00_801D4FD0(index, (u8)(record->level + 1));
  }
  if (value != -1) {
    sprintf((char*)D_80145AD4, D_801D0C34, value);
  } else {
    sprintf((char*)D_80145AD4, D_801D0C38);
  }
  func_8014FF0C((s16)(x + 0x15), (s16)(y + 0x1A), 0, D_80145AD4);
  func_801AF390((s16)x, (s16)y, D_801D8984, 1);
}
