#include "bof3/bof3.h"

extern int rand(void);

/* 0x8015477C, the shared distance helper of the main executable: it takes a
 * signed 16.16 word pair as (x, y) and returns the pair's distance as a signed
 * halfword. The per-record sweep of this target publishes the same result in
 * the angle halfword of the record at 0x800E4800 it draws. */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* The weight table this entry walks to pick its record: eight byte weights that
 * sum to 0x40, the range of the rand() draw taken below, so the walk always
 * stops on an entry below 8. It sits in the single data blob claimed by the
 * textbin row at D_801F364C. */
extern u8 D_801F37E8[];

/* The position-pair table the picked record is seeded from: eight entries of an
 * x byte and a y byte, both scaled by 0x10000 into the record's position words.
 * It sits in the same data blob, 0x10 bytes below the weight table. */
typedef struct Area12513PositionPair {
  u8 x;
  u8 y;
} Area12513PositionPair;

extern Area12513PositionPair D_801F37D8[];

/* The shared 0x98-byte work records at 0x80146888, seen as the eight slots this
 * entry chooses between. Only the fields it writes are modelled: the lead flag
 * byte it clears on every record it does not pick, the two position words and
 * the distance halfword. */
typedef struct Area12513WorkRecord {
  u8 flags_00;
  u8 unk_01[0x33];
  s32 position_34;
  s32 position_38;
  u8 unk_3C[2];
  s16 distance_3E;
  u8 unk_40[0x58];
} Area12513WorkRecord;

extern Area12513WorkRecord D_80146888[];

/* Shared halfwords of the main executable: the cell this entry reads and the
 * cell it writes that value minus 5 into. Neither address is inside this
 * overlay's own image, so both stay bound by address. */
extern u16 D_80145FB8[];
extern u16 D_80143F22;

/* @source 0x801F3504
 * @behavior Weighted record pick of this overlay: it draws rand() into the low
 *           six bits and walks the eight byte weights at 0x801F37E8,
 *           subtracting every weight the draw does not fall below, so the walk
 *           stops on the weighted record index it selects. It then clears the
 *           lead flag byte at offset 0 of every one of the first eight
 *           0x98-byte records at 0x80146888 except the selected one, and for a
 *           selected index below 8 it draws rand() again, takes the low three
 *           bits of that draw as the index of the eight byte-pair entries at
 *           0x801F37D8, stores the entry's x byte shifted left 16 in the
 *           record's position word at +0x34 and its y byte the same way in the
 *           word at +0x38, then stores the distance helper func_8015477C of
 *           those two words in the signed halfword at +0x3E. Finally it stores
 *           the halfword at 0x80145FB8 minus 5 in the shared halfword at
 *           0x80143F22 and returns nothing; its 0x20-byte frame keeps $ra and
 *           $s0/$s1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWeightedRandomWorkRecordClearOthersPositionAndDistanceThenStoreSharedHalfwordMinus5World03Area12513_801F3504(
    void) {
  u32 roll;
  s32 slot;
  s32 i;
  s32 pick;

  roll = rand();
  roll &= 0x3F;
  for (slot = 0; slot < 8; slot++) {
    if ((roll & 0xFF) < D_801F37E8[slot]) {
      break;
    }
    roll -= D_801F37E8[slot];
  }

  for (i = 0; i < 8; i++) {
    if (i != slot) {
      D_80146888[i].flags_00 = 0;
    }
  }

  if (slot < 8) {
    pick = rand();
    D_80146888[slot].position_34 = D_801F37D8[pick & 7].x << 16;
    D_80146888[slot].position_38 = D_801F37D8[pick & 7].y << 16;
    D_80146888[slot].distance_3E = func_8015477C(
        D_80146888[slot].position_34, D_80146888[slot].position_38);
  }

  D_80143F22 = D_80145FB8[0] - 5;
}
