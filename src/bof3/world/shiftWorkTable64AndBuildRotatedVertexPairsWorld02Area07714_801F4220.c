#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x800E5500 @kind unknown */
extern u16 workTableHalfword800E5500;
/* @source 0x800E5502 @kind unknown */
extern u8 workTableByte800E5502;

/* @source 0x80143E6C @kind data
 * Shared per-frame counter in the main executable. The seeded entry takes its
 * low seven bits, negated, as the angle the local angle lookup below reads.
 */
extern u32 D_80143E6C;

/* 0x801783C8 and 0x801782FC - the main executable's angle pair. Both take a
 * 16-bit angle and return the 0x10000-scaled offset the entry centre below
 * scales and adds to the entry position. */
s32 func_801783C8(u16 angle);
s32 func_801782FC(u16 angle);

/* One 0x34-byte entry of the 64-entry work table at 0x800E4800. The three
 * leading words hold the entry position, the two vertex triples the ribbon
 * edge pair the drawer 0x801F43C8 projects, and the trailing halfword the
 * twist angle that edge pair is rotated by. */
typedef struct WorkTable64Record {
  s32 position_x_00;
  s32 position_y_04;
  s32 position_z_08;
  s32 unk_0c;
  s32 vertex_a_x_10;
  s32 vertex_a_y_14;
  s32 vertex_a_z_18;
  s32 unk_1c;
  s32 vertex_b_x_20;
  s32 vertex_b_y_24;
  s32 vertex_b_z_28;
  s32 unk_2c;
  u16 angle_30;
  u16 unk_32;
} WorkTable64Record;

/* @source 0x801F4220
 * @behavior Ages the 64-entry work table at 0x800E4800 by one entry: it
 *           copies entries 0..62 into entries 1..63 in descending order, and
 *           seeds entry 0 from the scratch work record published through
 *           0x1F800044, taking work words +0x34/+0x38 as the entry x/y and the
 *           work word +0x3C plus the 0x801782FC offset of the negated low seven
 *           bits of the shared frame counter 0x80143E6C, shifted left ten, as
 *           the entry z. Entry 0 then takes the current twist halfword 0x800E5500
 *           as its twist angle, and that halfword advances by 0x40 while the
 *           gate byte 0x800E5502 is set, the gate clearing once the halfword
 *           wraps to zero. Finally every entry's vertex pair is rewritten as
 *           x = entry x, y = entry y plus or minus the 0x801783C8 offset of the
 *           twist angle shifted left three, and z = entry z plus or minus the
 *           0x801782FC offset of the same angle shifted left eleven, so the two
 *           edges straddle the entry position at the entry twist. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void shiftWorkTable64AndBuildRotatedVertexPairsWorld02Area07714_801F4220(void) {
  WorkTable64Record* record;
  u8* work;
  s32 angle_offset;
  s32 edge_y_span;
  s32 edge_z_span;
  s32 entry_z;
  u16 twist;
  u16 next_twist;
  u8 twist_enabled;
  u8 i;

  record = PSX_PTR(WorkTable64Record, 0x800E5498u);
  i = 0;
  do {
    record[1] = *record;
    record--;
    i++;
  } while (i < 0x3F);

  record = PSX_PTR(WorkTable64Record, 0x800E4800u);
  work = workRecordCursor1F800044;
  record->position_x_00 = *(s32*)(work + 0x34);
  record->position_y_04 = *(s32*)(work + 0x38);
  angle_offset = func_801782FC((u16)~(D_80143E6C << 7));
  work = workRecordCursor1F800044;
  twist = workTableHalfword800E5500;
  twist_enabled = workTableByte800E5502;
  entry_z = *(s32*)(work + 0x3C) + (angle_offset << 10);
  record->angle_30 = twist;
  record->position_z_08 = entry_z;
  if (twist_enabled != 0) {
    next_twist = twist + 0x40;
    workTableHalfword800E5500 = next_twist;
    if (next_twist == 0) {
      workTableByte800E5502 = 0;
    }
  }
  i = 0;
  do {
    edge_y_span = func_801783C8(record->angle_30) << 3;
    edge_z_span = func_801782FC(record->angle_30) << 11;
    record->vertex_a_x_10 = record->position_x_00;
    record->vertex_a_y_14 = record->position_y_04 + edge_y_span;
    record->vertex_a_z_18 = record->position_z_08 + edge_z_span;
    record->vertex_b_x_20 = record->position_x_00;
    record->vertex_b_y_24 = record->position_y_04 - edge_y_span;
    record->vertex_b_z_28 = record->position_z_08 - edge_z_span;
    record++;
    i++;
  } while (i < 0x40);
}
