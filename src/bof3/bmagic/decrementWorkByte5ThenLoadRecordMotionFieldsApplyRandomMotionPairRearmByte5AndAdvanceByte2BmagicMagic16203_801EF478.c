#include "bof3/bof3.h"

/* Pointer cell at 0x801F17B0 in this overlay's trailing data block. It holds
 * the current 0x2C-byte bmagic work record, published by the overlay frame loop
 * at 0x801EEC28. This body is entry 6 of the stage handler table at 0x801EEC04
 * and re-reads the cell before each access group, so the record pointer is never
 * live across a call. The address sits inside the trailing `textbin` row
 * labelled D_801F00B8, so the reference binds through this target's generated
 * undefined-symbol table and needs no map row. */
extern u8 *D_801F17B0; /* @source 0x801F17B0 @kind data */

/* Shared battle record published at 0x801EB4E0, outside this overlay's image.
 * The exact sibling copyWorkTripletSeedOffsetPairAndAdvanceWorkByte2Bmagic
 * Magic10503_801EF06C reads the same cell and copies the same-style word
 * triple out of it, so the word and halfword roles below stay structural and
 * the name stays address-anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Per-entry motion offset pair table at 0x801F00B8. Each entry is two halfwords
 * with a four-byte stride, so the entry address is the base plus the record
 * byte at 0x7 masked with 0x1F, shifted left twice. On disc the entries are
 * multiples of eight and the first halfword of some entries is negative, which
 * is the sign this body tests. */
struct BmagicMotionPair {
  s16 dx;
  u16 dy;
};
extern struct BmagicMotionPair D_801F00B8[]; /* @source 0x801F00B8 @kind data */

/* Byte table at 0x801F01A4 indexed by the work record entry byte at 0x4; the
 * branch reseeds the record byte at 0x7 from it. No lifted source names it yet,
 * so it stays an address-anchored byte array. */
extern u8 D_801F01A4[]; /* @source 0x801F01A4 @kind data */

/* PsyQ random generator at 0x8017E3D4, declared the way the exact bmagic
 * siblings declare it; it is bound through the generated undefined-funcs row. */
extern int rand(void);

/* @source 0x801EF478
 * @behavior Stage handler 6 of the nine-entry pointer run at 0x801EEC04
 *           (0x801EED38, 0x801EEFA0, 0x801EF048, 0x801EF158, 0x801EF1A0,
 *           0x801EF2D8, 0x801EF478, 0x801EF678, 0x801EF728) of this overlay:
 *           it decrements the work record countdown byte at 0x5 with a
 *           byte-width read-modify-write and returns while the stored value
 *           stays non-zero. Only when that countdown reaches zero does it
 *           reload the record motion fields from the shared battle record at
 *           0x801EB4E0 - the three words at 0x34, 0x38 and 0x3C into the record
 *           words at 0x14, 0x18 and 0x1C and the halfwords at 0x2E and 0x30
 *           into the record halfwords at 0x20 and 0x22 - and then apply a
 *           randomized motion offset pair. The per-entry pair table at
 *           0x801F00B8 is indexed by the record byte at 0x7 masked with 0x1F;
 *           a negative first halfword of that entry subtracts the random nibble
 *           of rand from the record halfword at 0x20 and a non-negative one
 *           adds it, and the entry second halfword plus that random nibble is
 *           subtracted from the record halfword at 0x22. The accumulated
 *           halfword at 0x20 is copied into the record halfword at 0x8, fresh
 *           rand results masked with 1 and unmasked seed the halfwords at 0xA
 *           and 0xC, the record byte at 0x7 is reseeded with the byte table at
 *           0x801F01A4 indexed by the record byte at 0x4 plus a random result
 *           masked with 6, the countdown byte at 0x5 is re-armed with 0x10, the
 *           byte at 0x6 is cleared and the stage byte at 0x2 is advanced. It
 *           takes no arguments, returns nothing and keeps a 0x18-byte frame
 *           only for $ra.
 * @status partial
 * @match 94.53
 * @residual Frame, size, guard, field-copy and randomized-pair groups are
 *           instruction-identical. The record halfword copy from 0x20 into
 *           0x8 and the halfword at 0x22 update are emitted in the opposite
 *           order and share one record-pointer cell load: the original
 *           re-loads the cell at 0x801F17B0 between those two stores, while
 *           this candidate keeps the first load live because the halfword
 *           store is not treated as clobbering the cell. Every probe that
 *           forces a second cell read (volatile cell, an explicit record local
 *           per statement, reordered statements) measured worse, so the shape
 *           is retained for review.
 */
void decrementWorkByte5ThenLoadRecordMotionFieldsApplyRandomMotionPairRearmByte5AndAdvanceByte2BmagicMagic16203_801EF478(
    void) {
  if (--D_801F17B0[5] == 0) {
    ((u32 *)D_801F17B0)[5] = ((u32 *)D_801EB4E0)[0xD];
    ((u32 *)D_801F17B0)[6] = ((u32 *)D_801EB4E0)[0xE];
    ((u32 *)D_801F17B0)[7] = ((u32 *)D_801EB4E0)[0xF];
    ((u16 *)D_801F17B0)[0x10] = ((u16 *)D_801EB4E0)[0x17];
    ((u16 *)D_801F17B0)[0x11] = ((u16 *)D_801EB4E0)[0x18];
    if (D_801F00B8[((u8 *)D_801F17B0)[7] & 0x1F].dx < 0) {
      ((u16 *)D_801F17B0)[0x10] +=
          (u16)D_801F00B8[((u8 *)D_801F17B0)[7] & 0x1F].dx - (rand() & 7);
    } else {
      ((u16 *)D_801F17B0)[0x10] +=
          (u16)D_801F00B8[((u8 *)D_801F17B0)[7] & 0x1F].dx + (rand() & 7);
    }
    ((u16 *)D_801F17B0)[4] = ((u16 *)D_801F17B0)[0x10];
    ((u16 *)D_801F17B0)[0x11] -=
        D_801F00B8[((u8 *)D_801F17B0)[7] & 0x1F].dy + (rand() & 7);
    ((u16 *)D_801F17B0)[5] = rand() & 1;
    ((u16 *)D_801F17B0)[6] = rand();
    ((u8 *)D_801F17B0)[7] =
        D_801F01A4[((u8 *)D_801F17B0)[4]] + (rand() & 6);
    ((u8 *)D_801F17B0)[5] = 0x10;
    ((u8 *)D_801F17B0)[6] = 0;
    ((u8 *)D_801F17B0)[2]++;
  }
}
