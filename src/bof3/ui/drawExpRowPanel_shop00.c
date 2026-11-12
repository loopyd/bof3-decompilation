#include "bof3/core/slus_internal.h"
#include "bof3/ui/shop00_internal.h"

/* Sprite-record stream of this EMI payload, at payload byte 0x14A80: the
 * 0x5C-byte run ends with the 0x00FF0000 terminator packet, so it is submitted
 * whole to func_801AF390. It follows the 0x5C-byte stream D_801E5674 and its
 * role is only recovered as "the stream this panel submits", so it keeps the
 * generated name the asm reference had.
 * @source 0x801E5680 @kind unknown */
extern const u8 D_801E5680[];

/* "EXP" and "NEXT" captions of this panel's two modes, in the payload rodata run
 * that also holds the row captions D_801E5828 ("Def"), D_801E582C ("Int"),
 * D_801E5830 ("Agl") and the "EQUIP" caption D_801E583C: the mode byte selects
 * between them. The names stay the generated ones because only the text, not
 * its owner, is recovered.
 * @source 0x801E5844 @kind rodata */
extern const char D_801E5844[];
/* @source 0x801E5848 @kind rodata */
extern const char D_801E5848[];

/* Decimal format and eight-dash placeholder of this payload's leading data
 * blob, at payload bytes 0xF8 and 0xFC: the value row formats with "%8d" and
 * substitutes the dash run when the value reads -1.
 * @source 0x801D0CF8 @kind rodata */
extern const char D_801D0CF8[];
/* @source 0x801D0CFC @kind rodata */
extern const char D_801D0CFC[];

/* Caption drawer of the main executable: the two signed coordinates, the
 * alignment argument 0, the 0xFF mask and the caption address as the fifth
 * argument, which the original stores to the outgoing stack slot.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

extern int sprintf(char* buffer, const char* format, ...);

/* Value sum of this EMI, defined by func_801DAA78.c: the row and the number of
 * entries it adds, reporting -1 for a count the row cannot hold.
 * @source 0x801DAA78 */
extern s32 func_801DAA78(u8 row, u8 count);

/* One 164-byte entry of the record table D_80144968 selected by the index byte;
 * only the two fields this body reads are named.
 * @source 0x80144968 @kind table */
typedef struct ShopExpRowRecord {
  u8 unk_00[6];
  u8 level;  /* +0x06 level byte the value sum walks past */
  u8 unk_07; /* +0x07 */
  u32 exp;   /* +0x08 experience word the EXP caption shows */
  u8 unk_0C[0x98];
} ShopExpRowRecord;

/* @source 0x801D8C58
 * @behavior experience row panel of the SHOP overlay: it fills the 0x65 by
 *           0x25 inset rectangle at (x+3, y+3) through func_801AE3F0 with the
 *           shade 0 and the CLUT-bank byte D_80144952, then selects the
 *           164-byte record D_80144952[index*164+0x16] of the shared record
 *           table. With the mode byte zero it draws the EXP caption D_801E5844
 *           through func_8014F800 at (x+0x2C, y+7) with the mask 0xFF and takes
 *           the word at record+8 as its value; with the mode byte set it draws
 *           the NEXT caption D_801E5848 at (x+0x26, y+7) with the same mask and
 *           takes as its value the sum func_801DAA78 forms over the first
 *           record[6]+1 values of that record's row. It formats the value into
 *           the shared buffer D_80145AD4 with the format D_801D0CF8 when it is
 *           not -1 and with the placeholder D_801D0CFC when it is, draws that
 *           buffer through func_8014FF0C at (x+0x15, y+0x1A) with the shade 0
 *           and finally submits the sprite-record stream D_801E5680 at (x, y)
 *           through func_801AF390 with the flag 1. Nothing is returned and the
 *           0x50-byte frame keeps $s0 to $s5 and $ra across the calls. No jal
 *           and no word of this payload names the span, so its four arguments
 *           have the roles their uses give them: a caller outside the payload
 *           invokes it. One payload cmp proved these 0x1D4 bytes equal the
 *           emi/etc/bate/00 span at 0x801D31B0 and the emi/etc/shisu/00 span at
 *           0x801D4158 apart from ten words: the four jump target words at
 *           +0xA0, +0xD0, +0x100 and +0x144, each 0x5AA8 and 0x4B00 lower in
 *           those copies, the jal at +0x114, which names each overlay's own
 *           value sum, and the five literal low halves at +0xCC, +0xDC, +0x138,
 *           +0x158 and +0x1A0 (the two captions, the two format strings and the
 *           stream), while the lui halves stay equal - so this is one body with
 *           this overlay's own bindings and no byte-identical copy exists.
 * @status partial
 * @match 71.79
 * @residual the callee-saved pair of the mode byte and the y coordinate: this
 *           body keeps y in $s2 and mode in $s3 where the original keeps mode in
 *           $s2 and y in $s3, so the save pair at 0x38/0x3C, the two copies and
 *           every later use of those registers differ. The global-alloc dump of
 *           this body orders y (4 references, 2-byte quantity, 6 call crossings)
 *           ahead of mode (3 references, 1-byte quantity, 2 crossings) in the
 *           priority list, which fixes y to the lower register; the original must
 *           have ordered them the other way. Measured attempts to move mode ahead
 *           all kept that order or lost instructions: caption select as a
 *           statement if/else, as a u32 variable, hoisted before the record add,
 *           an index/coordinate/label s16 or u32 local, the two coordinates read
 *           through locals after the record add, s32 and u32 index with explicit
 *           byte masks, and the coordinate spelled with a cast per arm. The
 *           remaining differences follow from the same allocation: the original
 *           narrows x before the mode test and truncates each arm's sum before one
 *           shared sign extension, forms the record base in place with `addiu
 *           s0,s0,0x16`, and re-masks mode for its second test, while this body
 *           narrows x inside the test, reassociates the record add as
 *           `164*index + 0x16`, and reuses the first mask.
 */
void drawExpRowPanel_shop00(s16 x, s16 y, u8 index, u8 mode) {
  u8* base;
  ShopExpRowRecord* record;
  s32 value;
  const char* caption;
  u8 frame_reserve[0x18]; /* size-only frame reservation, see the note above */

  base = &D_80144952;
  func_801AE3F0((u16)(x + 3), (u16)(y + 3), 0x65, 0x25, 0, base[0]);
  record = (ShopExpRowRecord*)(base + 0x16) + index;
  caption = mode == 0 ? D_801E5844 : D_801E5848;
  func_8014F800((s16)(x + (mode == 0 ? 0x2C : 0x26)), (s16)(y + 7), 0, 0xFF,
                (u32)caption);
  if (mode == 0) {
    value = record->exp;
  } else {
    value = func_801DAA78(index, (u8)(record->level + 1));
  }
  if (value != -1) {
    sprintf((char*)D_80145AD4, D_801D0CF8, value);
  } else {
    sprintf((char*)D_80145AD4, D_801D0CFC);
  }
  func_8014FF0C((s16)(x + 0x15), (s16)(y + 0x1A), 0, D_80145AD4);
  func_801AF390((s16)x, (s16)y, D_801E5680, 1);
}
