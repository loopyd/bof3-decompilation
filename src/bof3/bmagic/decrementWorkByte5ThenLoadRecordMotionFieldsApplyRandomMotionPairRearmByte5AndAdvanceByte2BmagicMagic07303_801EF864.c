#include "bof3/bof3.h"

/* Pointer cell at 0x801F21F0, the last word of this payload. It holds the
 * 0x2C-byte bmagic work record being stepped: the overlay frame loop at
 * 0x801EEC28 publishes the selected record into this cell. Every access below
 * re-reads the cell, so the record pointer is never live across a store or a
 * call and the original re-materialises the cell address before each access
 * group. The cell sits inside the payload's trailing `textbin` row labelled
 * D_801F05A4, so the reference binds through this target's generated
 * undefined-symbol table and needs no map row. */
extern u8 *D_801F21F0; /* @source 0x801F21F0 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image,
 * bound through this target's generated undefined-symbol table. Its word
 * triplet at 0x34/0x38/0x3C is the motion state this body copies into the
 * record words at 0x14/0x18/0x1C, and its halfwords at 0x2E/0x30 seed the record
 * halfwords at 0x20/0x22. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Per-entry motion offset pair table at 0x801F05A4. Each entry is two halfwords
 * with a four-byte stride, so the entry address is the base plus the record
 * byte at 0x7 masked with 0x1F, shifted left twice. On disc seven of the
 * thirty-two first-halfword entries are negative and the second halfwords are
 * positive, which is the sign this body tests before choosing whether to
 * subtract or add the random low three bits of a rand result. */
struct BmagicMotionPair {
  s16 dx;
  u16 dy;
};
extern struct BmagicMotionPair D_801F05A4[]; /* @source 0x801F05A4 @kind data */

/* Byte table at 0x801F0664, indexed by the work record byte at 0x4; the tail of
 * this body reseeds the record byte at 0x7 from it. Its first two entries are 6
 * and 12, and the code-pointer run at 0x801F0668 follows it. */
extern u8 D_801F0664[]; /* @source 0x801F0664 @kind data */

/* PsyQ random generator at 0x8017E3D4, bound through this target's generated
 * undefined-funcs row and declared the way the other bmagic sources declare
 * it. */
extern int rand(void);

/* @source 0x801EF864
 * @behavior Step handler of this overlay's stage walk, the first of the three
 *           code pointers at 0x801EEC1C (0x801EF864, 0x801EFA64, 0x801EFB14)
 *           that the dispatcher at 0x801EF768 copies into its own frame and
 *           indexes with the work record byte at offset 2, so it runs at step
 *           index 0. It decrements the work record countdown byte at 0x5 with a
 *           byte-width read-modify-write and returns while the stored value
 *           stays non-zero. Only when that countdown reaches zero does it reload
 *           the record motion fields from the shared battle record at
 *           0x801EB4E0 - the words at 0x34, 0x38 and 0x3C into the record words
 *           at 0x14, 0x18 and 0x1C and the halfwords at 0x2E and 0x30 into the
 *           record halfwords at 0x20 and 0x22 - and apply a randomized motion
 *           offset pair. The per-entry pair table at 0x801F05A4 is indexed by
 *           the record byte at 0x7 masked with 0x1F; a negative first halfword
 *           of that entry subtracts the low three bits of a rand result from the
 *           record halfword at 0x20 and a non-negative one adds them, and that
 *           entry's second halfword plus those bits is subtracted from the
 *           record halfword at 0x22. The accumulated halfword at 0x20 is then
 *           copied into the record halfword at 0x8, fresh rand results masked
 *           with 1 and unmasked seed the halfwords at 0xA and 0xC, the record
 *           byte at 0x7 is reseeded with the byte table at 0x801F0664 indexed by
 *           the record byte at 0x4 plus a rand result masked with 6, the
 *           countdown byte at 0x5 is re-armed with 0x10, the byte at 0x6 is
 *           cleared and the step byte at 0x2 is advanced. It takes no arguments,
 *           returns nothing, keeps a 0x18-byte frame only for $ra and reads or
 *           writes no other state. The roles of countdown byte 5, step byte 2 and
 *           record bytes 4, 6 and 7 are not established, so they stay
 *           offset-anchored. The 128 instructions of this span are
 *           instruction-identical to the span at 0x801EF478 of the bmagic 162
 *           overlay payload except for the four data-symbol immediates and that
 *           span's absolute jump word, so that near-identical sibling supplied
 *           the statement order, operand forms, offset spelling and declaration
 *           style while every literal and symbol binding here comes from this
 *           span's own bytes. A comparison of this 512-byte span against all 396
 *           extracted payloads under out/binaries, verbatim and with the
 *           data-symbol immediates masked, found no twin, so no sibling is a
 *           clone source.
 * @status partial
 * @match 94.53
 * @residual 121 of 128 instructions and all 512 bytes match. The countdown
 *           guard, the shared-record reload, both randomized motion arms, the
 *           record byte 7/5/6/2 rearm group and the motion field copies are
 *           instruction-identical. The one differing group is the record
 *           halfword copy from 0x20 into 0x8: the original re-materialises the
 *           pointer cell at 0x801F21F0 after the randomized pair store to 0x22
 *           and emits the copy behind it, filling the delay slot of the next
 *           rand call, while this candidate emits that copy one call earlier.
 *           Measured alternatives: the two statements in the original order
 *           (116 of 128, 12 bytes short, the copy hoisted into the pair group
 *           and sharing its cell load), the motion pair offset held in a local
 *           (107 of 128), and a jitter local, the expanded assignment, swapped
 *           addition operands, pointer-arithmetic spelling and a copy local,
 *           all at 121 of 128 or unchanged. The same body in the bmagic 162
 *           payload is partial at 94.53 on the same seven instructions under
 *           its own schedule, so the residual is a scheduling choice of this
 *           compiler rather than a spelling gap.
 */
void decrementWorkByte5ThenLoadRecordMotionFieldsApplyRandomMotionPairRearmByte5AndAdvanceByte2BmagicMagic07303_801EF864(
    void) {
  if (--D_801F21F0[5] == 0) {
    ((u32 *)D_801F21F0)[5] = ((u32 *)D_801EB4E0)[0xD];
    ((u32 *)D_801F21F0)[6] = ((u32 *)D_801EB4E0)[0xE];
    ((u32 *)D_801F21F0)[7] = ((u32 *)D_801EB4E0)[0xF];
    ((u16 *)D_801F21F0)[0x10] = ((u16 *)D_801EB4E0)[0x17];
    ((u16 *)D_801F21F0)[0x11] = ((u16 *)D_801EB4E0)[0x18];
    if (D_801F05A4[((u8 *)D_801F21F0)[7] & 0x1F].dx < 0) {
      ((u16 *)D_801F21F0)[0x10] +=
          (u16)D_801F05A4[((u8 *)D_801F21F0)[7] & 0x1F].dx - (rand() & 7);
    } else {
      ((u16 *)D_801F21F0)[0x10] +=
          (u16)D_801F05A4[((u8 *)D_801F21F0)[7] & 0x1F].dx + (rand() & 7);
    }
    ((u16 *)D_801F21F0)[4] = ((u16 *)D_801F21F0)[0x10];
    ((u16 *)D_801F21F0)[0x11] -=
        D_801F05A4[((u8 *)D_801F21F0)[7] & 0x1F].dy + (rand() & 7);
    ((u16 *)D_801F21F0)[5] = rand() & 1;
    ((u16 *)D_801F21F0)[6] = rand();
    ((u8 *)D_801F21F0)[7] = D_801F0664[((u8 *)D_801F21F0)[4]] + (rand() & 6);
    ((u8 *)D_801F21F0)[5] = 0x10;
    ((u8 *)D_801F21F0)[6] = 0;
    ((u8 *)D_801F21F0)[2]++;
  }
}
