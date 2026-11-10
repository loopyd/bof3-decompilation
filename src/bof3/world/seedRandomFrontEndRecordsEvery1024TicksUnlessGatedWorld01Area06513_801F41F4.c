#include "bof3/bof3.h"

/* @source 0x80146871 @kind data */
extern u8 D_80146871;
/* @source 0x80143E6C @kind data */
extern u32 D_80143E6C;
typedef struct Area06513FrontEndRecord {
  u8 unk_00;
  u8 unk_01;
  u8 unk_02[3];
  u8 unk_05;
  u8 unk_06;
  u8 unk_07;
  u8 unk_08;
  u8 unk_09[0x35];
  s16 unk_3E;
  u8 unk_40[0x34];
} Area06513FrontEndRecord;

/* @source 0x80143FC8 @kind data */
extern Area06513FrontEndRecord D_80143FC8[];
/* @source 0x80145EC4 @kind data */
extern s32 D_80145EC4[];
/* @source 0x80196070 */
extern void func_80196070(void);
/* @source 0x8019601C */
extern u8 func_8019601C(void);
/* @source 0x8015477C */
extern s16 func_8015477C(s32 x, s32 y);
extern int rand(void);

/* @source 0x801F41F4
 * @behavior If bit 0 of shared byte 0x80146871 is set, calls 0x80196070.
 * Otherwise, when the low ten bits of shared tick word 0x80143E6C are zero,
 * takes one random value and claims up to its low two bits worth of records
 * through 0x8019601C, stopping on index 0xFF. For each 0x74-byte record at
 * 0x80143FC8, stores 1 at offsets 0 and 1, 0x16 at offset 5, the loop index
 * at offset 6, and random bits 4-5 at offset 8. Stores the signed result of
 * 0x8015477C on the shared word pair 0x80145EC4/0x80145EC8 plus 0x400 in
 * the halfword at offset 0x3E. Takes no arguments and returns nothing.
 * @status partial
 * @match 75.00
 * @residual Allocator sentinel hoisting, cursor-base allocation and loop-entry shape differ; 80 instructions versus 79 original.
 */
void seedRandomFrontEndRecordsEvery1024TicksUnlessGatedWorld01Area06513_801F41F4(void) {
  u8 index;
  s32 i;
  u32 value;
  u32 masked;
  u32 shaped;
  s32 count;
  s32* source;

  if ((D_80146871 & 1) != 0) {
    func_80196070();
    return;
  }
  if ((D_80143E6C & 0x3FF) != 0) {
    return;
  }
  value = (u32)rand();
  masked = value & 0x33;
  count = (s32)(value & 3);
  shaped = masked >> 4;
  source = D_80145EC4;
  for (i = 0; i < count; i++) {
    index = func_8019601C();
    if (index == 0xFF) {
      return;
    }
    D_80143FC8[index].unk_00 = 1;
    D_80143FC8[index].unk_05 = 0x16;
    D_80143FC8[index].unk_01 = 1;
    D_80143FC8[index].unk_06 = i;
    D_80143FC8[index].unk_08 = shaped;
    D_80143FC8[index].unk_3E = func_8015477C(source[0], source[1]) + 0x400;
  }
}
