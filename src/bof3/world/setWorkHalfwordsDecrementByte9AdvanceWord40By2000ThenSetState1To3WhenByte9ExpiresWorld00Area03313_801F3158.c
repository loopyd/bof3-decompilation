#include "bof3/bof3.h"

typedef struct Area03313WorkRecord {
  u8  unk_00;
  u8  mode;
  u8  unk_02[0x07];
  u8  unk_09;
  u8  unk_0a[0x36];
  u32 unk_40;
  u8  unk_44[0x04];
  u8  unk_48;
} Area03313WorkRecord;

/* @source 0x1F800044 @kind unknown */
extern Area03313WorkRecord* D_1F800044;

/* The local seeder writes 0xA0 to work offset 0x2E and 0x50 to offset 0x30 and
 * takes no arguments. */
void setWorkHalfwordsWorld00Area03313_801F3334(void);

void func_8014D4E0(void);

/* @source 0x801F3158
 * @behavior Seeds the work halfwords through the local 0x801F3334 helper, which
 *           writes 0xA0 to work offset 0x2E and 0x50 to offset 0x30, then
 *           decrements the countdown byte at 0x09 of the record published at
 *           scratchpad pointer cell 0x1F800044 and advances the word at 0x40 by
 *           0x2000. It then reads that cell again and, once the countdown has
 *           reached zero, clears work byte 0x48 and writes 3 to the state byte
 *           at 0x01 through a third read of the cell. Finally it calls the
 *           shared resident helper at 0x8014D4E0. Takes no arguments, uses no
 *           result, and its 0x18-byte frame holds the return address only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfwordsDecrementByte9AdvanceWord40By2000ThenSetState1To3WhenByte9ExpiresWorld00Area03313_801F3158(
    void) {
  Area03313WorkRecord* work;
  Area03313WorkRecord* state;

  setWorkHalfwordsWorld00Area03313_801F3334();
  work = D_1F800044;
  work->unk_09--;
  work->unk_40 += 0x2000;

  state = D_1F800044;
  if (state->unk_09 == 0) {
    state->unk_48 = 0;
    D_1F800044->mode = 3;
  }

  func_8014D4E0();
}
