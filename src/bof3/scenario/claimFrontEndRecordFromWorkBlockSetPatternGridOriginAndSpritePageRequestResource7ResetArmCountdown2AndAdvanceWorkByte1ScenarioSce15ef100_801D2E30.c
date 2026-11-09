#include "bof3/bof3.h"

/* Scratchpad cell that publishes the current work object. The body reads the
 * cell three times, before the block copy, before it publishes the fresh
 * record and again for the work byte 1 advance, because calls and stores
 * separate those accesses.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Progress byte of the shared main executable. The body only claims a record
 * while this byte reads 0x29; the caller that sets that value is outside this
 * payload, so the role stays address-anchored.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Front-end record table of the shared main executable, rows of 0x74 bytes.
 * The exact lift at 0x801FCD14 documents the same table through the field
 * bases of its rows, and func_8019601C answers the row index.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];

/* Selector byte of this payload that picks the pattern-grid entry whose base
 * pair becomes the record origin. The body writes 1 into it and then reads it
 * back as the index, so the store and the read are separate accesses.
 * @source 0x801D40AC @kind data
 */
extern u8 D_801D40AC;

/* Eight-byte pattern entries based at 0x801D393C inside this payload's data
 * run at 0x801D3918. Only the base pair at +0x00/+0x02 is read here, the
 * layout the exact lift at 0x801D30AC declares for the same table.
 * @source 0x801D393C @kind data
 */
typedef struct PatternGridEntry {
  u16 baseX; /* 0x00 */
  u16 baseY; /* 0x02 */
  s16 bound; /* 0x04 */
  u16 step;  /* 0x06 */
} PatternGridEntry;

extern PatternGridEntry D_801D393C[];

/* Front-end record block: the 0x74-byte row this body copies whole from the
 * work object. The cast form tells the compiler the row is word aligned, which
 * is what the original word copy shows. */
typedef struct FrontEndRecordBlock {
  u32 words[0x1D];
} FrontEndRecordBlock;

/* Record-index query of the shared main executable. It answers the unsigned
 * byte index of a free front-end record and 0xFF when none is free, the form
 * the exact lift at 0x801FCD14 documents for the same callee.
 * @source 0x8019601C
 */
extern u8 func_8019601C(void);

/* Shared main-executable seed helper. The body passes the fixed
 * 0x340/0x100/0x80/0x100 tuple and ignores the answer; the boss lift at
 * 0x800C1A78 declares the same four argument roles for the same callee.
 * @source 0x8014E564
 */
extern void func_8014E564(s16 x, s16 y, s16 w, s16 h);

/* Shared resource-request helper of the main executable. The body asks for
 * resource 7 and reads no answer, the same call form the plchar lift at
 * 0x801D0208 documents for the same callee.
 * @source 0x8014D6B8
 */
extern void func_8014D6B8(u32 id);

/* Shared no-argument service of the main executable. The body calls it once
 * between the resource request and the reset helper and reads no result; the
 * callers elsewhere disagree on its argument list, so the empty list stays
 * file-local to this caller.
 * @source 0x8014D290
 */
extern void func_8014D290(void);

/* Resident reset helper of the main executable. This overlay's neighbours and
 * the exact scenario lift at 0x801D0EC4 call it with no arguments and read no
 * result.
 * @source 0x80196070
 */
extern void func_80196070(void);

/* @source 0x801D2E30
 * @behavior Step handler 5 of the eleven-word code-pointer run at 0x801D397C
 *           that the dispatcher 0x801D2DEC indexes with work byte 1, so it is
 *           reached through that run alone and takes no arguments. While the
 *           shared progress byte reads 0x29 it asks func_8019601C for a free
 *           front-end record index and, unless that index reads 0xFF, writes 1
 *           into the payload selector byte 0x801D40AC, forms the 0x74-byte
 *           record at 0x80143FC8 + index * 0x74, calls func_8014E564 with
 *           0x340/0x100/0x80/0x100, copies the 0x74-byte block that the work
 *           object's word 0x4C points at into that record, and then seeds the
 *           record: byte 0x00 = 1, byte 0x01 = 2, byte 0x02 = 0, byte 0x05 =
 *           0x3A, the halfwords 0x2E/0x30 from the base pair of the
 *           pattern-grid entry that byte 0x801D40AC selects, byte 0x25 from
 *           GetTPage(0, 0, 0x340, 0x100), byte 0x26 = 0x80 and byte 0x24 |=
 *           0x88. It then publishes the fresh record through the scratchpad
 *           cell, requests main-executable resource 7, runs func_8014D290 and
 *           the reset helper func_80196070 while the fresh record is the
 *           published one, restores the cell to the previous work object and
 *           arms that object's countdown byte 0x09 with 2. Either way it then
 *           reads the cell back and increments work byte 0x01. The roles of
 *           the selector byte, of the copied block and of resource 7 are not
 *           established here, so the name describes the observed step. The
 *           0x20-byte frame only keeps $ra and the three saved registers across
 *           the calls, every arm joins on one epilogue, and the slots after the
 *           uncached loads and after the call words that have no usable
 *           follower stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape note: the unlifted sibling func_801D21B4 at 0x801D21B4, entry 0 of the
 * same run, holds the same claim, the same 0x74-byte block copy, the same four
 * record seed bytes, the same pattern-grid origin pair and the same
 * publish/request/reset/restore sequence with extra guarded arms this body does
 * not have, so it is shape evidence rather than a seed; the copy loop, the
 * expression forms and the declaration style carry over with this function's
 * own address, its own 0x29 progress gate and its own resource 7 request.
 *
 * Shape note from the original bytes: the three cell reads are three separate
 * accesses, which is why only the value restored to the cell is kept in a local
 * across the calls. The block source read and the work byte 0x01 advance read
 * the cell in place: the first dies in the copy and the second is the one cell
 * load the advance body shares between the byte read and the store. Holding
 * either in the same local as the restored value made the compiler keep one
 * register across the calls and cost the pair its separate load, measured
 * 93/106 with the copy read wrong and 88/106 with the advance read wrong.
 */
void claimFrontEndRecordFromWorkBlockSetPatternGridOriginAndSpritePageRequestResource7ResetArmCountdown2AndAdvanceWorkByte1ScenarioSce15ef100_801D2E30(
    void) {
  u8* record;
  u8* previous;
  u8* source;
  u8  index;

  if (g_ScenarioProgress == 0x29) {
    index = func_8019601C();
    if (index != 0xFFu) {
      D_801D40AC = 1;
      record = D_80143FC8 + (u32)index * 0x74u;
      func_8014E564(0x340, 0x100, 0x80, 0x100);

      source = *(u8**)(D_1F800044 + 0x4C);
      *(FrontEndRecordBlock*)record = *(FrontEndRecordBlock*)source;

      record[0] = 1;
      record[1] = 2;
      record[2] = 0;
      record[5] = 0x3A;
      *(u16*)(record + 0x2E) = D_801D393C[D_801D40AC].baseX;
      *(u16*)(record + 0x30) = D_801D393C[D_801D40AC].baseY;
      record[0x25] = GetTPage(0, 0, 0x340, 0x100);
      record[0x26] = 0x80;
      record[0x24] |= 0x88;

      previous = D_1F800044;
      D_1F800044 = record;
      func_8014D6B8(7);
      func_8014D290();
      func_80196070();
      D_1F800044 = previous;
      previous[9] = 2;
    }

    D_1F800044[1] = D_1F800044[1] + 1;
  }
}
