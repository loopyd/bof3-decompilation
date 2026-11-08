#include "bof3/bof3.h"

/* Front-end entry table at 0x80143FC8: 0x74-byte records. Only the fields this
 * step touches are proven for this target: the occupancy byte at 0x00, the
 * handler selector byte at 0x05 (the selector of the table at 0x801C7C70), the
 * byte at 0x29, the two halfwords at 0x2E and 0x30 that this step clears, and
 * the byte at 0x0B that receives the owner work-record index. */
typedef struct Area17213FrontEndRecord {
  u8  flags_00;
  u8  unk_01[4];   /* 0x01 - 0x04 */
  u8  handler_05;
  u8  unk_06[5];   /* 0x06 - 0x0A */
  u8  owner_0B;
  u8  unk_0C[0x1D]; /* 0x0C - 0x28 */
  u8  unk_29;
  u8  unk_2A[4];   /* 0x2A - 0x2D */
  u16 unk_2E;
  u16 unk_30;
  u8  unk_32[0x42]; /* 0x32 - 0x73 */
} Area17213FrontEndRecord;

/* Work records published through the pointer cell 0x80146884 and based at
 * 0x80146888. Only the record stride is proven here: the original divides the
 * two pointers with a typed difference over a 0x98-byte cell, which the
 * compiler emits as the exact-division shift-add chain for the modular inverse
 * of 19 followed by a three-bit arithmetic shift. */
typedef struct Area17213WorkRecord {
  u8 bytes[0x98];
} Area17213WorkRecord;

extern Area17213FrontEndRecord D_80143FC8[]; /* @source 0x80143FC8 @kind table */
extern Area17213WorkRecord* D_80146884;      /* @source 0x80146884 @kind unknown */
extern Area17213WorkRecord D_80146888[];     /* @source 0x80146888 @kind bss */
s32 func_8019601C(void);

/* @source 0x801F49DC
 * @behavior Asks the shared free-record selector func_8019601C for an index,
 *           masks the result to a byte and, unless that byte is 0xFF, claims
 *           the selected 0x74-byte front-end record at 0x80143FC8: marks it
 *           occupied (byte 0x00 = 1), writes handler selector 0x92 into byte
 *           0x05, writes 4 into byte 0x29, clears the halfwords at 0x2E and
 *           0x30, and stores the index of the current work record - the
 *           0x80146884 minus 0x80146888 pointer difference over the 0x98-byte
 *           work-record stride - into byte 0x0B. Takes no arguments and
 *           returns nothing; its only in-image reference is word 6 of the
 *           pointer run based at 0x801F6708, which holds 0x801F4B3C,
 *           0x801F4324, 0x801F434C, 0x801F43C8, 0x801F471C and 0x801F491C
 *           before it and 0x801F4AAC and 0x801F4AF4 after it.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The record stays written as an indexed table row rather than a
 * materialised row pointer: the original keeps `index * 0x74` in $a0 and
 * folds each store into `lui $at`/`addu $at,$at,$a0` against the table base.
 */
void claimFrontEndRecordSetHandler92AndStoreOwnerWorkIndexWorld04Area17213_801F49DC(
    void) {
  s32 index;

  index = func_8019601C() & 0xFF;
  if (index != 0xFF) {
    D_80143FC8[index].flags_00 = 1;
    D_80143FC8[index].handler_05 = 0x92;
    D_80143FC8[index].unk_29 = 4;
    D_80143FC8[index].unk_2E = 0;
    D_80143FC8[index].unk_30 = 0;
    D_80143FC8[index].owner_0B = (u8)(D_80146884 - D_80146888);
  }
}
