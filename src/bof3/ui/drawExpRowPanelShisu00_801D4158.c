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
 * blob, at payload bytes 0x18 and 0x1C: the value row uses the first, and the
 * second replaces it when the value reads -1.
 * @source 0x801D0C18 @kind data */
extern const char D_801D0C18[];
/* @source 0x801D0C1C @kind data */
extern const char D_801D0C1C[];

/* Caption strings of this overlay's trailing data blob, eight and twelve bytes
 * past the six-byte equip id table: the mode byte selects between them.
 * @source 0x801D9B6C @kind data */
extern const char D_801D9B6C[];
/* @source 0x801D9B70 @kind data */
extern const char D_801D9B70[];

/* Sprite-record stream of this overlay's trailing data blob, submitted at the
 * panel origin with the flag byte 1.
 * @source 0x801D99A8 @kind data */
extern const u8 D_801D99A8[];

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

/* Experience-table sum of this overlay, defined by sumLevelExpShisu00_801D5F78.c:
 * the character row and the number of level entries it adds. */
extern s32 sumLevelExpShisu00_801D5F78(u8 row, u8 count);

/* One 164-byte entry of the shared party-record table selected by the index
 * byte; only the fields this body consumes are named. */
typedef struct ShisuExpRowRecord {
  u8 unk_00[6];
  u8 level;  /* +0x06 level byte the experience sum walks past */
  u8 unk_07; /* +0x07 */
  u32 exp;   /* +0x08 experience word the EXP caption shows */
  u8 unk_0C[0x98];
} ShisuExpRowRecord;

/* @source 0x801D4158
 * @behavior Experience row panel of the SHISU overlay: it fills the 0x65 by
 *           0x25 inset rectangle at (x+3, y+3) through func_801AE3F0 with the
 *           shade 0 and the CLUT-bank byte D_80144952[0], then selects the
 *           164-byte party record D_80144952[index*164+0x16]. With the mode
 *           byte zero it draws the EXP caption D_801D9B6C through func_8014F800
 *           at (x+0x2C, y+7) with the mask 0xFF and takes the word at
 *           record+8 as its value; with the mode byte set it draws the NEXT
 *           caption D_801D9B70 at (x+0x26, y+7) with the same mask and takes as
 *           its value the sum sumLevelExpShisu00_801D5F78 forms over the first
 *           record[6]+1 level entries of that character row. It formats the
 *           value into the shared buffer D_80145AD4 with the format D_801D0C18
 *           when it is not -1 and with the placeholder D_801D0C1C when it is,
 *           draws that buffer through func_8014FF0C at (x+0x15, y+0x1A) with
 *           the shade 0 and finally submits the sprite stream D_801D99A8 at
 *           (x, y) through func_801AF390 with the flag 1. Nothing is returned
 *           and the 0x50-byte frame keeps $s0 to $s5 and $ra across the calls.
 *           No jal and no word of this payload names the span, so its four
 *           arguments have the roles their uses give them: a caller outside the
 *           payload invokes it. One payload cmp proved these 0x1D4 bytes equal
 *           the drawExpRowPanelBate00_801D31B0 span at 0x801D31B0 in
 *           emi/etc/bate/00 apart from ten words - the five jump and jal target
 *           words of the body, each 0xFA8 above that copy's own, and the five
 *           literal low halves at +0xCC, +0xDC and +0x1A0 (the two captions and
 *           the sprite stream) and at +0x138 and +0x158 (the two format
 *           strings), while the lui halves stay equal - and the same ten words
 *           are the only difference from the emi/etc/shop/00 span at 0x801D8C58,
 *           each 0x4B00 below this one, so this is one body with this overlay's
 *           own bindings.
 * @status partial
 * @match 71.79
 * @residual the two remaining callee-saved registers of the byte pair: this
 *           body keeps the mode byte in $s3 where the original keeps it in $s2
 *           and the y coordinate in $s2 where the original keeps it in $s3, so
 *           the prologue save order, the caption select and every later use of
 *           those two registers differ, and the original also splits the
 *           coordinate shift across the two arms of the mode test where this
 *           body shifts once after them, and fills four delay slots this body
 *           fills with real work. The 0x50-byte frame, the record pointer in
 *           $s0, the index byte in $s4, the x argument copy before the first
 *           call in $s1 and its copy after that call in $s5 agree.
 *
 * Frame note: the 0x50-byte frame carries 0x18 bytes that no emitted
 * instruction reads or writes, so they are the size-only reservation
 * `frame_reserve` below; only its size is constrained and its name and type are
 * a reconstruction choice.
 */
void drawExpRowPanelShisu00_801D4158(s16 x, s16 y, u8 index, u8 mode) {
  u8* base;
  ShisuExpRowRecord* record;
  s32 value;
  const char* caption;
  u8 frame_reserve[0x18]; /* size-only frame reservation */

  base = &D_80144952;
  func_801AE3F0((u16)(x + 3), (u16)(y + 3), 0x65, 0x25, 0, base[0]);
  record = (ShisuExpRowRecord*)(base + 0x16) + index;
  caption = mode == 0 ? D_801D9B6C : D_801D9B70;
  func_8014F800((s16)(x + (mode == 0 ? 0x2C : 0x26)), (s16)(y + 7), 0, 0xFF,
                (u32)caption);
  if (mode == 0) {
    value = record->exp;
  } else {
    value = sumLevelExpShisu00_801D5F78(index, (u8)(record->level + 1));
  }
  if (value != -1) {
    sprintf((char*)D_80145AD4, D_801D0C18, value);
  } else {
    sprintf((char*)D_80145AD4, D_801D0C1C);
  }
  func_8014FF0C((s16)(x + 0x15), (s16)(y + 0x1A), 0, D_80145AD4);
  func_801AF390((s16)x, (s16)y, D_801D99A8, 1);
}
