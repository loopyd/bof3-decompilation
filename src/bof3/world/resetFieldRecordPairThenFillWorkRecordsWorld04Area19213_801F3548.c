#include "bof3/bof3.h"

/* One 164-byte field record of the D_80144968 array. The byte at 0x07 is the
 * flag that gates the reset, the halfword at 0x0C is cleared by it and the
 * halfword pair at 0x1C/0x1E feeds the pair at 0x14/0x16. */
typedef struct {
  u8 unk_00[7];    /* 0x00 */
  u8 unk_07;       /* 0x07 */
  u32 unk_08;      /* 0x08 */
  u16 unk_0C;      /* 0x0C */
  u8 unk_0E[6];    /* 0x0E */
  u16 unk_14;      /* 0x14 */
  u16 unk_16;      /* 0x16 */
  u32 unk_18;      /* 0x18 */
  u16 unk_1C;      /* 0x1C */
  u16 unk_1E;      /* 0x1E */
  u8 unk_20[0x84]; /* 0x20 */
} Area19213FieldRecord;

/* One 320-byte work record of the 0x140-byte-stride array whose base is
 * 0x80145E90, addressed from its 0x74 interior base: the field-record copy
 * occupies the first 164 bytes of the element. */
typedef struct {
  Area19213FieldRecord field; /* 0x000 */
  u8 unk_A4[0x9C];            /* 0x0A4 */
} Area19213WorkRecord;

extern Area19213FieldRecord D_80144968[]; /* @source 0x80144968 @kind table */
extern Area19213WorkRecord D_80145F04[];  /* @source 0x80145F04 @kind table */
extern u8 D_80144F5A[];                   /* @source 0x80144F5A @kind table */
extern u8 D_80181B10[];                   /* @source 0x80181B10 @kind table */
extern u8 D_80146254;                     /* @source 0x80146254 @kind unknown */

/* @source 0x801F3548
 * @behavior Walks the eight 164-byte field records of D_80144968 and, for
 * every record whose flag byte at offset 0x07 has bit 0 set, clears the
 * halfword at offset 0x0C and copies the halfwords at 0x1C and 0x1E over the
 * pair at 0x14 and 0x16. It then walks the visible entry count D_80146254 in
 * order, selects the field record index D_80181B10[D_80144F5A[entry]] and
 * copies that whole 164-byte field record into offset 0x74 of the entry work
 * record in the 0x140-byte-stride array this row addresses from its 0x74
 * interior base D_80145F04 (array base D_80145E90). The count is re-read after
 * every copy, so the global is read once per loop entry rather than hoisted.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the 304-byte span at payload offset
 * 0x948 is byte-identical to emi/world04/area191/13 0x801F3AB0, which is
 * already lifted exact, so the body is a hand copy of that source with this
 * target's record typedef names; the clone adapter dry run reported the same
 * 304-byte identity and rewrote no symbol, because every global the body
 * touches encodes to the same address in both overlays. The single caller at
 * 0x801F2F54 in the mode table entry 0x801F2E98 passes no arguments, matching
 * this body's empty parameter list.
 */
void resetFieldRecordPairThenFillWorkRecordsWorld04Area19213_801F3548(void) {
  s32 i;

  for (i = 0; i < 8; i++) {
    if ((D_80144968[i].unk_07 & 1) != 0) {
      D_80144968[i].unk_0C = 0;
      D_80144968[i].unk_14 = D_80144968[i].unk_1C;
      D_80144968[i].unk_16 = D_80144968[i].unk_1E;
    }
  }
  for (i = 0; i < D_80146254; i++) {
    D_80145F04[i].field = D_80144968[D_80181B10[D_80144F5A[i]]];
  }
}
