#include "bof3/bof3.h"

/* Scratchpad cell that publishes the current work object. The body touches it
 * five times: for the block source, for the value it restores after the
 * publish, for the fresh-record store itself, for the byte 0x2A clear of the
 * untaken resource arm and for the work byte 1 advance at the end, because
 * calls and stores separate those accesses.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record table of the shared main executable, rows of 0x74 bytes.
 * The exact lift at 0x801D2E30 documents the same table through the field
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
 * layout the exact lifts at 0x801D2E30 and 0x801D30AC declare for the same
 * table.
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
 * the exact lift at 0x801D2E30 documents for the same callee.
 * @source 0x8019601C
 */
extern u8 func_8019601C(void);

/* Shared main-executable seed helper. The body passes the fixed
 * 0x340/0x100/0x80/0x100 tuple and ignores the answer, the same call form the
 * exact lift at 0x801D2E30 keeps for this callee.
 * @source 0x8014E564
 */
extern void func_8014E564(s16 x, s16 y, s16 w, s16 h);

/* Shared main-executable resource request helper with a resource id and a
 * count. This body reaches the pair form twice, (0x50, 0x24) and (0x4D,
 * 0x1C); the exact plchar lift at 0x801CE628 declares the same two-argument
 * shape for the same callee.
 * @source 0x8014D6DC
 */
extern void func_8014D6DC(s32 id, s32 count);

/* Shared no-argument service of the main executable. The body calls it once
 * between the resource request and the reset helper and reads no result.
 * @source 0x8014D290
 */
extern void func_8014D290(void);

/* Resident reset helper of the main executable. This overlay's neighbours and
 * the exact scenario lift at 0x801D0EC4 call it with no arguments and read no
 * result.
 * @source 0x80196070
 */
extern void func_80196070(void);

/* @source 0x801D21B4
 * @behavior Entry 0 of both work byte 1 code-pointer runs of this payload, the
 *           ten-word run at 0x801D3954 that the dispatcher 0x801D2170 indexes
 *           and the eleven-word run at 0x801D397C that the dispatcher
 *           0x801D2DEC indexes, so it is reached through those runs alone and
 *           takes no arguments. It asks func_8019601C for a free front-end
 *           record index and, unless that index reads 0xFF, writes 1 into the
 *           payload selector byte 0x801D40AC, forms the 0x74-byte record at
 *           0x80143FC8 + index * 0x74, calls func_8014E564 with
 *           0x340/0x100/0x80/0x100, copies the 0x74-byte block that the work
 *           object word 0x4C points at into that record, and then seeds the
 *           record: byte 0x00 = 1, byte 0x01 = 2, byte 0x02 = 0, byte 0x05 =
 *           0x3A, the halfwords 0x2E/0x30 from the base pair of the
 *           pattern-grid entry that byte 0x801D40AC selects, and, only while
 *           record byte 0x24 still reads odd, byte 0x25 from
 *           GetTPage(0, 0, 0x340, 0x100) with byte 0x26 = 0x80, after which
 *           byte 0x24 |= 0x88. It then publishes the fresh record through the
 *           scratchpad cell and, only while record byte 0x24 reads odd, asks
 *           for resource (0x50, 0x24) when the halfword at record +0x2C is
 *           nonzero and for resource (0x4D, 0x1C) plus a clear of record byte
 *           0x2A otherwise. Then it runs func_8014D290 and the reset helper
 *           func_80196070 while the fresh record is the published one,
 *           restores the cell to the previous work object and arms that
 *           object's countdown byte 0x09 with 2. Either way it then reads the
 *           cell back and increments work byte 0x01. The roles of the selector
 *           byte, of the copied block and of the two resource ids are not
 *           established here, so the name describes the observed step. The
 *           0x20-byte frame only keeps $ra and the three saved registers across
 *           the calls, every arm joins on one epilogue, and the slots after the
 *           uncached loads and after the call words that have no usable
 *           follower stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape note: the exact sibling 0x801D2E30 at 0x801D2E30 holds the same claim,
 * the same 0x74-byte block copy, the same four record seed bytes, the same
 * pattern-grid origin pair and the same publish/reset/restore/advance sequence,
 * and it differs in the arms the original bytes of this span show: this body
 * carries no shared progress gate, guards the page seed and the resource
 * request on record byte 0x24 bit 0 instead, and requests resource 0x50 or
 * 0x4D where the sibling requests resource 7. The copy loop, the expression
 * forms and the declaration style carry over with this function's own call
 * words, its own guards and its own resource ids.
 *
 * Shape note from the original bytes: the two record byte 0x24 tests are two
 * separate byte reads with the 0x88 store between them, and the resource arm
 * reloads the scratchpad cell rather than reusing the local, because the cell
 * holds the fresh record at that point and the original stores through it.
 */
void claimFrontEndRecordFromWorkBlockSeedPatternGridAndSpritePageRequestResource50Or4DResetArmCountdown2AndAdvanceWorkByte1ScenarioSce15ef100_801D21B4(
    void) {
  u8* record;
  u8* previous;
  u8* source;
  u8  index;

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

    if (record[0x24] & 1) {
      record[0x25] = GetTPage(0, 0, 0x340, 0x100);
      record[0x26] = 0x80;
    }
    record[0x24] |= 0x88;

    previous = D_1F800044;
    D_1F800044 = record;
    if (record[0x24] & 1) {
      if (*(u16*)(record + 0x2C) == 0) {
        func_8014D6DC(0x4D, 0x1C);
        D_1F800044[0x2A] = 0;
      } else {
        func_8014D6DC(0x50, 0x24);
      }
    }
    func_8014D290();
    func_80196070();
    D_1F800044 = previous;
    previous[9] = 2;
  }

  D_1F800044[1] = D_1F800044[1] + 1;
}
