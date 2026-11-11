#include "bof3/bof3.h"

/* Shared party-record table of the main executable. This body reads its byte 0
 * as the CLUT-bank argument of the inset painter and forms the selected record
 * 0x16 bytes past that byte, so the leading byte is the cell named here.
 * @source 0x80144952 @kind data */
extern u8 D_80144952;

/* Shared sprintf work buffer of the main executable, outside this EMI payload:
 * each formatted value row is written here and then handed to the line-text
 * drawer as the buffer address.
 * @source 0x80145AD4 @kind data */
extern u8 D_80145AD4[];

/* Value format of this overlay's leading data blob, at payload byte 4: the
 * three-character decimal field "%3d" that all four value rows use.
 * @source 0x801D0C04 @kind data */
extern const char D_801D0C04[];

/* Caption rows of this overlay's trailing data blob, the four four-byte runs
 * starting "Pwr", "Def", "Int" and "Agl" at payload bytes 0x8F4C, 0x8F50,
 * 0x8F54 and 0x8F58: each is submitted as the fourth-cell run that labels one
 * value row of the attribute grid.
 * @source 0x801D9B4C @kind data */
extern const u8 D_801D9B4C[];
/* @source 0x801D9B50 @kind data */
extern const u8 D_801D9B50[];
/* @source 0x801D9B54 @kind data */
extern const u8 D_801D9B54[];
/* @source 0x801D9B58 @kind data */
extern const u8 D_801D9B58[];

/* Sprite-record streams of this overlay's trailing data blob: the panel
 * backing submitted once at the panel origin, the sixteen-cell row submitted
 * once per cell at x + cell*8 and the closing overlay submitted once at the
 * panel origin. All three carry the flag byte 0.
 * @source 0x801D9834 @kind data */
extern const u8 D_801D9834[];
/* @source 0x801D9860 @kind data */
extern const u8 D_801D9860[];
/* @source 0x801D986C @kind data */
extern const u8 D_801D986C[];

/* Inset rectangle painter of the main executable: the two coordinates, the
 * width, the height, the shade and the CLUT-bank byte. Both coordinates arrive
 * zero-extended here, so those are the call-site widths.
 * @source 0x801AE3F0 */
extern void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u8 arg5);

/* Text submitter of the main executable: the pen origin, the alignment, the
 * cell count and the glyph-code row it queues.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Line-text drawer of the main executable: the two signed coordinates, the
 * palette shade and the string it draws.
 * @source 0x80150098 */
extern void func_80150098(s16 arg0, s16 arg1, u32 arg2, const u8* arg3);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

/* Main-exe text handle query: returns the handle that the byte index selects,
 * which this body hands on as the text argument of func_8014F800.
 * @source 0x801502D0 */
extern u32 func_801502D0(u32 arg0);

/* Main-exe glyph-run attribute writer: the two constants 0 and 8 this body
 * passes, then the handle func_801502D0 returns for the badge byte.
 * @source 0x8016620C */
extern void func_8016620C(u32 arg0, u32 arg1, u32 arg2);

extern int sprintf(char* buffer, const char* format, ...);

/* One 164-byte entry of the shared party-record table selected by the index
 * byte; only the fields this body consumes are named. The four attribute
 * halfwords land in the grid in the caption order Pwr, Def, Int, Agl.
 */
typedef struct ShisuParameterPanelRecord {
  u8 unk_00[0x1B];  /* +0x00 */
  u8 badge;         /* +0x1B drawn as a badge unless it reads 0xFF */
  u16 unk_1C;       /* +0x1C */
  u16 unk_1E;       /* +0x1E */
  u16 power;        /* +0x20 formatted "%3d" beside the "Pwr" caption */
  u16 defense;      /* +0x22 beside "Def" */
  u16 agility;      /* +0x24 beside "Agl" */
  u16 intelligence; /* +0x26 beside "Int" */
  u8 unk_28[0x7C];  /* record tail, stride is 0xA4 */
} ShisuParameterPanelRecord;

/* @source 0x801D3BD4
 * @behavior Parameter panel of the SHISU overlay: it fills the 0x94 by 0x2D
 *   inset rectangle at (x+5, y+3) through func_801AE3F0 with the shade 0 and
 *   the CLUT-bank byte D_80144952[0], then selects the 164-byte party record
 *   0x16 bytes past that byte and fills a two-by-two attribute grid of it. The
 *   captions "Pwr", "Def", "Int" and "Agl" of D_801D9B4C, D_801D9B50,
 *   D_801D9B54 and D_801D9B58 are submitted through func_8014F800 at (x+0xB,
 *   y+8), (x+0xB, y+0x15), (x+0x53, y+8) and (x+0x53, y+0x15) with the mask
 *   0xFF, and the halfword each labels is formatted with "%3d" into the shared
 *   buffer D_80145AD4 and drawn through func_80150098 at (x+0x30, y+0xA),
 *   (x+0x30, y+0x17), (x+0x78, y+0xA) and (x+0x78, y+0x17) with the shade 0.
 *   When the record badge byte 0x1B does not read 0xFF it hands the handle
 *   func_801502D0 returns for that byte plus 0x111 to func_8016620C with the
 *   constants 0 and 8, and draws the handle func_801502D0 returns for 0x34 at
 *   (x+0xB, y+0x22) with the mask 0xFF. It closes with the sprite stream
 *   D_801D9834 at the panel origin, the stream D_801D9860 once per each of the
 *   sixteen cells at x + cell*8, and the stream D_801D986C at the panel origin,
 *   all through func_801AF390 with the flag byte 0. Nothing is returned and the
 *   0x48-byte frame keeps $s0 through $s7 and $fp beside $ra across the calls.
 *   No jal and no word of this payload names the span, so its three arguments
 *   have the roles their uses give them: a caller outside the payload invokes
 *   it. One payload cmp proved these 0x338 bytes equal the func_801D1444 span
 *   at 0x801D1444 in emi/etc/sisyou/00 apart from fourteen words - the lui and
 *   addiu halves of the four caption addresses and of the three stream
 *   addresses, each of which binds this payload's own blob - while every
 *   relative branch stays equal, so this is one body carrying this overlay's
 *   own bindings. That copy is still a partial lift there, so no sibling source
 *   supplied this body.
 * @status partial
 * @match 98.06
 * @residual two hunks. The first is one register in the record-base sum: the
 *   original adds the record header offset into the table base it already holds
 *   (addiu s2,s2,22) and then adds the scaled index (addu s0,s0,s2), where this
 *   body folds that offset into the scaled index first (addiu s0,s0,22), so the
 *   arithmetic and the instruction count agree and only the register that
 *   receives the 0x16 differs. The second is the badge byte test, where the
 *   original masks the loaded byte before the 0xFF compare - lbu a0,0x1B(s0); li
 *   v0,255; andi v1,a0,0xFF; beq v1,v0 - and keeps the unmasked load in a0 for
 *   the +0x111 handle argument, so it carries one instruction and four bytes
 *   this body does not: the compiler already knows the lbu result is eight bits
 *   wide and folds the redundant mask. Sixteen other spellings across seven
 *   sweeps did not move either hunk: the mask written in the compare, in a u32
 *   local's initializer, on a u8 local and through a cast pointer all fold, and
 *   the only spelling that keeps it - a volatile view of the badge byte - expands
 *   to 207 instructions with an unfilled load delay slot, moves the compare into
 *   v0 and is an unevidenced volatility claim on a cell this payload reads once,
 *   so it was declined. Binding the record base to its own pointer local, its
 *   own record-pointer local, a pre-offset table local or a mutated base moved
 *   the first difference earlier without changing the totals.
 */
void drawPartyMemberParameterPanelShisu00_801D3BD4(u16 x, u16 y, u8 index) {
  u8* base;
  ShisuParameterPanelRecord* record;
  u8 i;

  base = &D_80144952;
  func_801AE3F0((u16)(x + 5), (u16)(y + 3), 0x94, 0x2D, 0, base[0]);

  record = (ShisuParameterPanelRecord*)(base + 0x16) + index;

  func_8014F800((s16)(x + 0xB), (s16)(y + 8), 0, 0xFF, (u32)D_801D9B4C);
  sprintf((char*)D_80145AD4, D_801D0C04, record->power);
  func_80150098((s16)(x + 0x30), (s16)(y + 0xA), 0, D_80145AD4);

  func_8014F800((s16)(x + 0xB), (s16)(y + 0x15), 0, 0xFF, (u32)D_801D9B50);
  sprintf((char*)D_80145AD4, D_801D0C04, record->defense);
  func_80150098((s16)(x + 0x30), (s16)(y + 0x17), 0, D_80145AD4);

  func_8014F800((s16)(x + 0x53), (s16)(y + 8), 0, 0xFF, (u32)D_801D9B54);
  sprintf((char*)D_80145AD4, D_801D0C04, record->intelligence);
  func_80150098((s16)(x + 0x78), (s16)(y + 0xA), 0, D_80145AD4);

  func_8014F800((s16)(x + 0x53), (s16)(y + 0x15), 0, 0xFF, (u32)D_801D9B58);
  sprintf((char*)D_80145AD4, D_801D0C04, record->agility);
  func_80150098((s16)(x + 0x78), (s16)(y + 0x17), 0, D_80145AD4);

  if ((record->badge & 0xFF) != 0xFF) {
    func_8016620C(0, 8, func_801502D0(record->badge + 0x111));
    func_8014F800((s16)(x + 0xB), (s16)(y + 0x22), 0, 0xFF,
                  func_801502D0(0x34));
  }

  func_801AF390((s16)x, (s16)y, D_801D9834, 0);

  for (i = 0; i < 16; i++) {
    func_801AF390((s16)(x + i * 8), (s16)y, D_801D9860, 0);
  }

  func_801AF390((s16)x, (s16)y, D_801D986C, 0);
}
