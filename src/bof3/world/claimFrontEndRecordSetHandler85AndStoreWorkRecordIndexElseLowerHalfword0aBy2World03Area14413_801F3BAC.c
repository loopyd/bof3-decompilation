#include "bof3/bof3.h"

/* Front-end record table of the shared main executable: 0x74-byte rows based at
 * 0x80143FC8. Only the fields this step touches are proven here: the occupancy
 * byte at 0x00, the handler selector byte at 0x05 (the selector of the table at
 * 0x801C7C70) and the byte at 0x06 that receives the index of the current work
 * record. The original keeps `index * 0x74` in $a0 and folds each store into
 * `lui $at`/`addu $at,$at,$a0` against the table base, so the source keeps an
 * indexed table row rather than a materialised row pointer.
 * @source 0x80143FC8 @kind table
 */
typedef struct World03Area14413FrontEndRecord {
  u8  flags_00;
  u8  unk_01[4];    /* 0x01 - 0x04 */
  u8  handler_05;
  u8  unk_06;
  u8  unk_07[0x6D]; /* 0x07 - 0x73 */
} World03Area14413FrontEndRecord;

/* Work records of the shared main executable, based at 0x80146888 and published
 * through the pointer cell 0x80146884. Only the stride is proven here: the
 * original derives the index of the current work record from the typed
 * difference between that pointer and the table base, whose exact-division
 * shift-add chain is the one this compiler emits for a 0x98-byte element, and
 * the record's halfword 0x0A is the countdown the neighbouring step
 * 0x801F3AEC advances by 3.
 * @source 0x80146888 @kind bss
 */
typedef struct World03Area14413WorkRecord {
  u8 bytes[0x98];
} World03Area14413WorkRecord;

extern World03Area14413FrontEndRecord D_80143FC8[];
/* @source 0x80146884 @kind unknown */
extern World03Area14413WorkRecord* D_80146884;
extern World03Area14413WorkRecord D_80146888[];
/* Record published through the global pointer cell at 0x80148210. The original
 * reads and writes its halfword 0x0A through one loaded address, so the cell is
 * declared at halfword width here. */
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* Record allocator of the shared EXE image, outside this payload: it answers the
 * index of a free 0x74-byte row of the front-end record table, or 0xFF when
 * every row is occupied. The original masks the answer to a byte before the
 * 0xFF test and before the row chain.
 */
s32 func_8019601C(void);

/* @source 0x801F3BAC
 * @behavior Overlay handler reached from the handler table word at 0x801F490C,
 *           entry 6 of the word run based at 0x801F48F4 that also holds
 *           0x801F3818, 0x801F3A20, 0x801F3AB4, 0x801F3AEC, 0x801F3B24,
 *           0x801F3C78, 0x801F3CE4 and 0x801F3D14. It asks the shared
 *           free-record selector func_8019601C for an index and masks the
 *           answer to a byte. Unless that byte reads 0xFF it claims the
 *           selected 0x74-byte front-end record of the shared table at
 *           0x80143FC8: marks it occupied (record byte 0x00 = 1), writes
 *           handler selector 0x85 into record byte 0x05 and stores into record
 *           byte 0x06 the index of the current work record, the 0x80146884
 *           minus 0x80146888 pointer difference over the 0x98-byte work-record
 *           stride. When the selector answers 0xFF instead it lowers the work
 *           record halfword 0x0A published at 0x80148210 by 2. It takes no
 *           arguments, returns nothing and its 0x18-byte frame saves $ra at
 *           0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordSetHandler85AndStoreWorkRecordIndexElseLowerHalfword0aBy2World03Area14413_801F3BAC(
    void) {
  s32 index;

  index = func_8019601C() & 0xFF;
  if (index != 0xFF) {
    D_80143FC8[index].flags_00 = 1;
    D_80143FC8[index].handler_05 = 0x85;
    D_80143FC8[index].unk_06 = (u8)(D_80146884 - D_80146888);
  } else {
    D_80148210[5] -= 2;
  }
}
