#include "bof3/bof3.h"

typedef struct Area12114WorkRecord {
  u8  unk_00;
  u8  mode;
  u8  unk_02[0x07];
  u8  unk_09;
  u8  unk_0a[0x36];
  u32 unk_40;
  u8  unk_44[0x04];
  u8  unk_48;
} Area12114WorkRecord;

/* @source 0x1F800044 @kind unknown */
extern Area12114WorkRecord* area12114WorkCursor_1F800044;

void setWorkHalfwordsWorld03Area12114_801F3418(void);
void func_8014D4E0(void);

/* @source 0x801F323C
 * @behavior Seeds the work halfwords through the local 0x801F3418 helper, which
 * writes 0xA0 to work offset 0x2E and 0x50 to offset 0x30, then decrements the
 * countdown byte at 0x09 and advances the word at 0x40 by 0x2000. It then reads
 * the scratchpad pointer cell 0x1F800044 again and, once that countdown has
 * reached zero, clears work byte 0x48 and writes 3 to the state byte at 0x01
 * through a third read of the cell. Finally it calls the shared resident helper
 * at 0x8014D4E0. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfwordsDecrementByte9AdvanceWord40By2000ThenSetState1To3WhenByte9ExpiresWorld03Area12114_801F323C(
    void) {
  Area12114WorkRecord* work;
  Area12114WorkRecord* state;

  setWorkHalfwordsWorld03Area12114_801F3418();
  work = area12114WorkCursor_1F800044;
  work->unk_09--;
  work->unk_40 += 0x2000;

  state = area12114WorkCursor_1F800044;
  if (state->unk_09 == 0) {
    state->unk_48 = 0;
    area12114WorkCursor_1F800044->mode = 3;
  }

  func_8014D4E0();
}
