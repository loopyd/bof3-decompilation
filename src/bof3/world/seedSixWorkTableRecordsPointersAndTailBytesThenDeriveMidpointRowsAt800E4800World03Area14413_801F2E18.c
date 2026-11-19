#include "bof3/bof3.h"

/* Main-RAM work words this bootstrap stores, five pointer words and the byte at
 * +0x1A of each of the six 0x1C-byte-stride records based at 0x800E4800. The
 * word destinations are the `sw` sites of the original span; the byte
 * destinations are its `sb` sites. Nothing outside this target names them, so
 * they keep their address spelling.
 */
/* @source 0x800E4800 @kind data */
extern u32 D_800E4800;
/* @source 0x800E4804 @kind data */
extern u32 D_800E4804;
/* @source 0x800E4808 @kind data */
extern u32 D_800E4808;
/* @source 0x800E480C @kind data */
extern u32 D_800E480C;
/* @source 0x800E4810 @kind data */
extern u32 D_800E4810;
/* @source 0x800E481A @kind data */
extern u8 D_800E481A;
/* @source 0x800E481C @kind data */
extern u32 D_800E481C;
/* @source 0x800E4820 @kind data */
extern u32 D_800E4820;
/* @source 0x800E4824 @kind data */
extern u32 D_800E4824;
/* @source 0x800E4828 @kind data */
extern u32 D_800E4828;
/* @source 0x800E482C @kind data */
extern u32 D_800E482C;
/* @source 0x800E4836 @kind data */
extern u8 D_800E4836;
/* @source 0x800E4838 @kind data */
extern u32 D_800E4838;
/* @source 0x800E483C @kind data */
extern u32 D_800E483C;
/* @source 0x800E4840 @kind data */
extern u32 D_800E4840;
/* @source 0x800E4844 @kind data */
extern u32 D_800E4844;
/* @source 0x800E4848 @kind data */
extern u32 D_800E4848;
/* @source 0x800E4852 @kind data */
extern u8 D_800E4852;
/* @source 0x800E4854 @kind data */
extern u32 D_800E4854;
/* @source 0x800E4858 @kind data */
extern u32 D_800E4858;
/* @source 0x800E485C @kind data */
extern u32 D_800E485C;
/* @source 0x800E4860 @kind data */
extern u32 D_800E4860;
/* @source 0x800E4864 @kind data */
extern u32 D_800E4864;
/* @source 0x800E486E @kind data */
extern u8 D_800E486E;
/* @source 0x800E4870 @kind data */
extern u32 D_800E4870;
/* @source 0x800E4874 @kind data */
extern u32 D_800E4874;
/* @source 0x800E4878 @kind data */
extern u32 D_800E4878;
/* @source 0x800E487C @kind data */
extern u32 D_800E487C;
/* @source 0x800E4880 @kind data */
extern u32 D_800E4880;
/* @source 0x800E488A @kind data */
extern u8 D_800E488A;
/* @source 0x800E488C @kind data */
extern u32 D_800E488C;
/* @source 0x800E4890 @kind data */
extern u32 D_800E4890;
/* @source 0x800E4894 @kind data */
extern u32 D_800E4894;
/* @source 0x800E4898 @kind data */
extern u32 D_800E4898;
/* @source 0x800E489C @kind data */
extern u32 D_800E489C;
/* @source 0x800E48A6 @kind data */
extern u8 D_800E48A6;

/* @source 0x801F376C @kind function */
extern void
storeMidpointOfTwoPointRecordsIntoThirdWorld03Area14413_801F376C(u8 arg0,
                                                                 u8 arg1,
                                                                 u8 arg2);

/* @source 0x801F2E18
 * @behavior Overlay step-0 bootstrap of the main-RAM work table at 0x800E4800,
 *           run once by the step-0 handler 0x801F2C48 (`jal` at 0x801F2C70),
 *           which is the only reference to this span: no other `jal` and no
 *           pointer word of the payload names it. It has two halves.
 *
 *           First a bottom-tested loop walks the six 0x1C-byte-stride records
 *           with an unsigned 16-bit counter and the record pointer anchored at
 *           0x800E481B, storing into record offsets 0x18 (1), 0x19 (the counter
 *           times 15, so 0x00, 0x0F, 0x1E, 0x2D, 0x3C, 0x4B), 0x17 (0), 0x14,
 *           0x15 and 0x16 (0xF0 each) in that order, and then into 0x1B: 3 when
 *           the counter is even and 1 when it is odd. The six stores precede the
 *           parity test, whose byte store is the loop pointer's own value.
 *
 *           Then, once per record in ascending record order, it stores five
 *           pointer words into record offsets 0x00 through 0x10 and the byte at
 *           0x1A around a call to the midpoint helper 0x801F376C; the byte, and
 *           the 0x10 word, follow that call for records 3 and 5, as the notes
 *           below record. The helper is called with the row triples
 *           (0,5,8), (0,3,9), (2,4,10), (2,7,11), (4,7,12) and (3,5,13), which
 *           derives rows 8 through 13 of this overlay's point table at
 *           0x801F3D80. The stored words point into the fourteen word cells
 *           0x800E48A8 through 0x800E48DC, addressed in cells 0..13 as record 0:
 *           0,1,4,5,8; record 1: 0,1,2,3,9; record 2: 2,0,6,4,10; record 3:
 *           2,3,6,7,11; record 4: 4,5,6,7,12; record 5: 3,1,7,5,13. The byte at
 *           0x1A is 1, 1, 7, 5, 5, 3 across the records in that order.
 *
 *           It takes no arguments and returns nothing. It reserves 0x40 bytes of
 *           frame with $ra at 0x3C and $s0 through $s6 at 0x20 through 0x38; the
 *           0x20 bytes below the saved registers hold nothing, because the loop
 *           pointer and the counter stay in argument registers. The stored
 *           pointer values are materialised as plain integer constants with
 *           lui/ori, so they are written as address casts rather than as the
 *           addresses of declared cells.
 *
 *           Representation notes: the unused 0x10-byte local restores the
 *           original's 0x40-byte frame. Without it the body reserves 0x30 and
 *           the first difference is the prologue's own `addiu $sp` (frame size
 *           and all eight save offsets), measured at 126 of 173 instructions;
 *           with it the frame and saves agree and the same shape measured 144 of
 *           173.
 *
 *           The parity store is written 3 for the even counter and 1 for the
 *           odd one, in that arm order: the original branches on the counter's
 *           raw low bit to the odd arm and leaves the even arm in the delay slot
 *           of the join jump, which the opposite arm order reproduces only as a
 *           negated `beqz` (measured as the first difference at 0x0060, 128 of
 *           173).
 *
 *           Two stores follow their record's midpoint call rather than
 *           preceding it, and the last record's fifth pointer word follows the
 *           byte at 0x1A: the original issues the 0x1A byte of record 3 after
 *           the call at 0x801F2FF8, and record 5's 0x1A byte and 0x10 word after
 *           the call at 0x801F307C, in that order. Store order is preserved
 *           across a call, so those three follow the source order the bytes show.
 *           The other four records' 0x1A bytes precede their calls, and record
 *           2's 0x10 word first among its five pointer words is the scheduler's
 *           own choice: writing it first in source measured 137 of 175
 *           instructions at 700 bytes, while the ascending spelling reproduces
 *           the original order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedSixWorkTableRecordsPointersAndTailBytesThenDeriveMidpointRowsAt800E4800World03Area14413_801F2E18(
    void) {
  /* Reserved frame only; no access and no use. Reconstruction choice. */
  u8 scratch[0x10];
  u8* work;
  u16 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    work[0x18] = 1;
    work[0x19] = i * 15;
    work[0x17] = 0;
    work[0x14] = 0xF0;
    work[0x15] = 0xF0;
    work[0x16] = 0xF0;
    if ((i & 1) == 0) {
      work[0x1B] = 3;
    } else {
      work[0x1B] = 1;
    }
    work += 0x1Cu;
    i += 1u;
  } while (i < 6u);

  D_800E4800 = (u32)PSX_PTR(void, 0x800E48A8u);
  D_800E4804 = (u32)PSX_PTR(void, 0x800E48ACu);
  D_800E4808 = (u32)PSX_PTR(void, 0x800E48B8u);
  D_800E480C = (u32)PSX_PTR(void, 0x800E48BCu);
  D_800E4810 = (u32)PSX_PTR(void, 0x800E48C8u);
  D_800E481A = 1;
  storeMidpointOfTwoPointRecordsIntoThirdWorld03Area14413_801F376C(0, 5, 8);

  D_800E481C = (u32)PSX_PTR(void, 0x800E48A8u);
  D_800E4820 = (u32)PSX_PTR(void, 0x800E48ACu);
  D_800E4824 = (u32)PSX_PTR(void, 0x800E48B0u);
  D_800E4828 = (u32)PSX_PTR(void, 0x800E48B4u);
  D_800E482C = (u32)PSX_PTR(void, 0x800E48CCu);
  D_800E4836 = 1;
  storeMidpointOfTwoPointRecordsIntoThirdWorld03Area14413_801F376C(0, 3, 9);

  D_800E4838 = (u32)PSX_PTR(void, 0x800E48B0u);
  D_800E483C = (u32)PSX_PTR(void, 0x800E48A8u);
  D_800E4840 = (u32)PSX_PTR(void, 0x800E48C0u);
  D_800E4844 = (u32)PSX_PTR(void, 0x800E48B8u);
  D_800E4848 = (u32)PSX_PTR(void, 0x800E48D0u);
  D_800E4852 = 7;
  storeMidpointOfTwoPointRecordsIntoThirdWorld03Area14413_801F376C(2, 4, 10);

  D_800E4854 = (u32)PSX_PTR(void, 0x800E48B0u);
  D_800E4858 = (u32)PSX_PTR(void, 0x800E48B4u);
  D_800E485C = (u32)PSX_PTR(void, 0x800E48C0u);
  D_800E4860 = (u32)PSX_PTR(void, 0x800E48C4u);
  D_800E4864 = (u32)PSX_PTR(void, 0x800E48D4u);
  storeMidpointOfTwoPointRecordsIntoThirdWorld03Area14413_801F376C(2, 7, 11);
  D_800E486E = 5;

  D_800E4870 = (u32)PSX_PTR(void, 0x800E48B8u);
  D_800E4874 = (u32)PSX_PTR(void, 0x800E48BCu);
  D_800E4878 = (u32)PSX_PTR(void, 0x800E48C0u);
  D_800E487C = (u32)PSX_PTR(void, 0x800E48C4u);
  D_800E4880 = (u32)PSX_PTR(void, 0x800E48D8u);
  D_800E488A = 5;
  storeMidpointOfTwoPointRecordsIntoThirdWorld03Area14413_801F376C(4, 7, 12);

  D_800E488C = (u32)PSX_PTR(void, 0x800E48B4u);
  D_800E4890 = (u32)PSX_PTR(void, 0x800E48ACu);
  D_800E4894 = (u32)PSX_PTR(void, 0x800E48C4u);
  D_800E4898 = (u32)PSX_PTR(void, 0x800E48BCu);
  storeMidpointOfTwoPointRecordsIntoThirdWorld03Area14413_801F376C(3, 5, 13);
  D_800E48A6 = 3;
  D_800E489C = (u32)PSX_PTR(void, 0x800E48DCu);
}
