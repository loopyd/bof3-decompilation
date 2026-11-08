#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so each load folds to lui plus an offset load, and because a store
 * through the loaded pointer may alias the cell the original re-reads it for
 * the pointer publication and again for the byte countdown instead of carrying
 * one fetched value through the block. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Battle state byte in main RAM, outside this overlay's image. The sibling boss
 * handlers of this module raise it to 3, 4 and 5; this handler reads it as the
 * record-array selector and as the index into the selected array, and re-reads
 * it after the size-0x50 helper instead of keeping the fetched byte live. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row is added here. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Local work records in main RAM, outside this overlay's image. The battle
 * lifts index the same array with a 0x140-byte stride and read its words
 * 0x34/0x38/0x3E/0x74; this handler only passes one record base to the
 * size-0x50 helper and to the range test, so the array stays byte-shaped here
 * and the stride is spelled out at the subscript. Splat binds the address
 * through this target's generated undefined_syms_auto.txt. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */

/* Battler work records in main RAM, outside this overlay's image. The battle
 * lifts index the same array with a 0x118-byte stride; this handler only passes
 * one record base to the size-0x50 helper and to the range test, so the array
 * stays byte-shaped here and the stride is spelled out at the subscript. Splat
 * binds the address through this target's generated undefined_syms_auto.txt. */
extern u8 D_801EB2E8[]; /* @source 0x801EB2E8 @kind unknown */

/* The 0x74-byte head of a work record: 29 words, copied as one block by the
 * original's seven 16-byte rounds plus the trailing word. Declared as a word
 * body so the block copy takes that shape. */
typedef struct Boss033WorkHead {
  u32 words_00[29];
} Boss033WorkHead;

/* One queued slot-store entry of the D_801EC330 array, 0x78 bytes. The battle
 * lift copySelectedWorkToQueuedSlot owns the same record layout in
 * include/bof3/battle/battle03_internal.h as Battle03SlotStore; this target
 * keeps its own declaration because it does not include the battle header, and
 * only the bytes this handler touches are named. */
typedef struct Boss033QueuedSlot {
  u8  unk_00;
  u8  unk_01;
  u8  unk_02;
  u8  unk_03;
  u8  unk_04;
  u8  unk_05;
  u8  unk_06;
  u8  pad_07[2];
  u8  unk_09;
  u8  pad_0a;
  u8  unk_0b;
  u8  pad_0c[0x68];
  u32 ptr_74;
} Boss033QueuedSlot;

extern Boss033QueuedSlot D_801EC330[]; /* @source 0x801EC330 @kind unknown */

/* Shared slot-store allocator of the battle executable, declared here with the
 * prototype the battle lift func_801E590C uses; the boss module passes the pair
 * (3, mode) and its sibling at 0x800C2340 passes (3, 5). Only the low byte of
 * the result is used, and 0xFF reports that no entry was allocated. */
extern u32 func_801E590C(u32 arg0, u32 arg1);

/* Record helper of the main executable, not lifted, so its effect stays
 * unestablished. The original passes a record base and the constant size 0x50 at
 * both call sites and re-reads the battle state byte afterwards. */
extern void func_800AF2B4(u8 *record, u32 size);

/* Record range test of the main executable. The battle lift func_800AF5FC owns
 * the body: it compares the scratch work object's 0x34/0x38/0x3E axes against
 * the passed record and returns non-zero while the object stays inside it. */
extern u32 func_800AF5FC(u8 *record, u32 value);

/* Shared main-executable resource request helper; the sibling boss and bmagic
 * lifts declare the same prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag);

/* Per-state handler of the main executable, not lifted; the original passes the
 * battle state byte it selects the record with. */
extern void func_800A9820(u8 arg0);

/* Readiness poll helper of the main executable. The battle and bmagic lifts
 * declare the same prototype; the original masks the returned byte with 0xFF
 * before testing it, so the declared return type is the byte it reports. */
extern u8 func_8014DAEC(void);

/* @source 0x800C20CC
 * @behavior Work-step handler at index 3 of this overlay's code-pointer run
 *           based at 0x800C27B4, the base word the step dispatcher
 *           func_800C1E70 indexes with work byte 2 scaled by four; run word 3 at
 *           0x800C27C0 holds this address and no jal in this payload names it.
 *           It first allocates a queued slot-store entry through
 *           func_801E590C(3, 6), keeps only the allocated index byte and writes
 *           nothing when that byte is 0xFF. Otherwise it copies the 0x74-byte
 *           head of the scratch work object published at the pointer cell
 *           0x1F800044 into the entry selected by that index at the 0x78-byte
 *           stride of D_801EC330, keeps the copied byte at entry offset 0 and
 *           sets bytes +0x01 = 1, +0x02 = 0, +0x03 = 0, +0x04 = 0, +0x05 = 6 and
 *           +0x06 = 3, then clears bit 0x40 of the kept byte and stores it back
 *           at offset 0. It publishes the scratch cell at entry offset +0x74,
 *           copies the work object byte 0x0B into entry byte +0x0B and, when
 *           that work byte is non-zero, decrements it by one. It then applies
 *           the main executable helper func_800AF2B4 to the record selected by
 *           the battle state byte 0x80146384 with the constant size 0x50: the
 *           local work record D_80145E90 at the 0x140-byte stride while the
 *           state byte is below three, otherwise the battler record D_801EB2E8
 *           at the 0x118-byte stride. When the range test func_800AF5FC, given
 *           that record and the constant 0x10000, reports the scratch work
 *           object inside the record range, it requests resource 5 through
 *           func_8014D6B8, runs the per-state handler func_800A9820 with the
 *           state byte and advances work byte 2, the step index, by one;
 *           otherwise that step index is untouched. It finishes with the
 *           readiness helper func_8014DAEC, whose returned byte it discards.
 *           Takes no arguments and returns nothing; the 0x20-byte frame only
 *           keeps the return address at 0x18 and the two callee-saved pointers
 *           at 0x14 and 0x10 across its calls. The roles of the allocation pair
 *           (3, 6), of the entry bytes +0x01..+0x06, of the work bytes 0x0B and
 *           2 beyond the displacement to the next run word, and of the helper
 *           size 0x50 are not established, so the name describes the measured
 *           effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkHeadIntoMode6SlotTickByteBThenApplyHelper50ToStateRecordRequestResource5WhenRangeCoversWorkAdvanceWorkByte2BossBoss03316_800C20CC(
    void) {
  Boss033QueuedSlot *record;
  Boss033WorkHead *head;
  u8 *work;
  u8 *state;
  u8 flags;
  u8 index;

  index = (u8)func_801E590C(3u, 6u);
  if (index != 0xFFu) {
    head = (Boss033WorkHead *)D_1F800044;
    record = &D_801EC330[index];
    *(Boss033WorkHead *)record = *head;
    flags = D_801EC330[index].unk_00;
    D_801EC330[index].unk_06 = 0x3;
    D_801EC330[index].unk_05 = 0x6;
    D_801EC330[index].unk_01 = 0x1;
    D_801EC330[index].unk_02 = 0x0;
    D_801EC330[index].unk_03 = 0x0;
    D_801EC330[index].unk_04 = 0x0;
    D_801EC330[index].unk_00 = flags & 0xBF;
    work = D_1F800044;
    D_801EC330[index].ptr_74 = (u32)work;
    D_801EC330[index].unk_0b = work[0xB];
    work = D_1F800044;
    if (work[0xB] != 0u) {
      work[0xB] -= 1u;
    }
  }
  state = &D_80146384;
  if (*state < 3u) {
    func_800AF2B4(D_80145E90 + *state * 0x140, 0x50);
    if (func_800AF5FC(D_80145E90 + *state * 0x140, 0x10000) != 0u) {
      func_8014D6B8(0x5);
      func_800A9820(*state);
      D_1F800044[2]++;
    }
  } else {
    func_800AF2B4(D_801EB2E8 + *state * 0x118, 0x50);
    if (func_800AF5FC(D_801EB2E8 + *state * 0x118, 0x10000) != 0u) {
      func_8014D6B8(0x5);
      func_800A9820(*state);
      D_1F800044[2]++;
    }
  }
  func_8014DAEC();
}
