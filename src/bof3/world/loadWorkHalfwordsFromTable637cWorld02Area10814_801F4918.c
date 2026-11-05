#include "bof3/bof3.h"

/* @source 0x80147B67 @kind unknown */
extern u8 D_80147B67;

/* Four-byte row of the work-row table at 0x801F637C. The original scales the
 * selector by four, and reads all four bytes of one row. */
typedef struct World02Area10814Row637c {
  u8 field_00;
  u8 field_01;
  u8 field_02;
  u8 field_03;
} World02Area10814Row637c;

/* @source 0x801F637C @kind table */
extern World02Area10814Row637c workRowTable637cWorld02Area10814[];

/* @source 0x801F4918
 * @behavior Copies the row selected by the 1-based byte at 0x80147B67 out of
 * the row table at 0x801F637C into the scratch work record published at
 * scratchpad pointer cell 0x1F800044: row byte 0 to halfword +0x36, row byte 2
 * to halfword +0x3A, and row bytes 1 and 3 each to a 0-or-0x8000 flag in
 * halfwords +0x34 and +0x38. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void loadWorkHalfwordsFromTable637cWorld02Area10814_801F4918(void) {
  u8* work;
  u32 index;

  index = (u8)(D_80147B67 - 1);
  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u16*)(work + 0x36) = workRowTable637cWorld02Area10814[index].field_00;
  *(u16*)(work + 0x34) =
      (u16)((workRowTable637cWorld02Area10814[index].field_01 != 0) << 15);
  *(u16*)(work + 0x3A) = workRowTable637cWorld02Area10814[index].field_02;
  *(u16*)(work + 0x38) =
      (u16)((workRowTable637cWorld02Area10814[index].field_03 != 0) << 15);
}
