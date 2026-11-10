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

/* The scratchpad pointer cell publishes the work record. The body loads it once
 * for the countdown store and the word retreat, a second time for the expiry
 * test and the 0x48 store, and a third time for the state byte store, so it is
 * read through this cell each time rather than through one cached pointer. */
extern Area03313WorkRecord* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Shared mode byte: the value 5 routes the countdown expiry into the shared
 * work-area reset instead of the in-place state re-arm. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* The local seeder at 0x801F3334 writes 0xA0 to work halfword 0x2E and 0x50 to
 * work halfword 0x30 and takes no arguments. */
void setWorkHalfwordsWorld00Area03313_801F3334(void);

/* Shared work-area reset at 0x80196070: clears the scratchpad game work-area
 * header bytes 0x00, 0x01, 0x02 and 0x03..0x04. */
void func_80196070(void);

/* Shared per-frame helper at 0x8014D4E0: the call site passes nothing and
 * consumes no result. */
void func_8014D4E0(void);

/* @source 0x801F3294
 * @behavior Seeds the work halfwords through the local 0x801F3334 helper, which
 *           writes 0xA0 to work offset 0x2E and 0x50 to offset 0x30, then
 *           decrements the countdown byte at 0x09 of the record published at
 *           scratchpad pointer cell 0x1F800044 and retreats the word at 0x40 by
 *           0x2000. It then reads that cell again and, once the countdown has
 *           reached zero, calls the shared work-area reset 0x80196070 when the
 *           shared mode byte 0x80143BB0 reads 5, and otherwise clears work byte
 *           0x48 and writes 1 to the work mode byte at 0x01 through a third read
 *           of the cell. While the countdown is still running it calls the
 *           shared per-frame helper 0x8014D4E0 instead. Takes no arguments,
 *           uses no result, and its 0x18-byte frame holds the return address
 *           only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementByte9RetreatWord40By2000ThenResetWorkAreaWhenSharedMode5OrSetState1OnByte9ExpiryWorld00Area03313_801F3294(
    void) {
  Area03313WorkRecord* work;
  Area03313WorkRecord* state;

  setWorkHalfwordsWorld00Area03313_801F3334();
  work = D_1F800044;
  work->unk_09--;
  work->unk_40 -= 0x2000;

  state = D_1F800044;
  if (state->unk_09 == 0) {
    if (D_80143BB0 == 5) {
      func_80196070();
    } else {
      state->unk_48 = 0;
      work = D_1F800044;
      work->mode = 1;
    }
  } else {
    func_8014D4E0();
  }
}
