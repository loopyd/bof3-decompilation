#include "bof3/bof3.h"

/* Scratchpad pointer cell holding this overlay's projected work record. The
 * cell is reloaded for every access, which is what the original bytes show. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Work-record pointer cell of the shared main executable, outside this
 * overlay's image: the tail of this step arms the record it publishes and then
 * re-reads the cell for the byte it stores. */
extern u8* D_80148210; /* @source 0x80148210 @kind data */

/* One projected position word, written whole by the projector at 0x801F3980.
 * Only its low halfword is read here; the high halfword is written and never
 * read by this step, so the word stays one scalar and the halfword is taken as
 * the second element of its halfword view. */
typedef s32 ProjectedWord;

/* Position projector of this overlay: it answers one byte and writes the two
 * projected position words through the pointers it is given. */
/* @source 0x801F3980 */
extern u8 func_801F3980(s32* x, s32* y);

/* Shared three-argument helper of the main executable. */
/* @source 0x80155038 */
extern void func_80155038(s16 x, s16 y, u32 size);

/* Shared map probe of the main executable: it answers the code byte of the
 * cell it is given as a column and a row. */
/* @source 0x80166CB0 */
extern s32 func_80166CB0(s16 col, s16 row);

/* Shared cue queue of the main executable. */
/* @source 0x801A4BC0 */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* Shared front-end arming helper of the main executable: it takes the work
 * record and the level byte to arm it with. */
/* @source 0x801ABBA8 */
extern void func_801ABBA8(u8* record, u8 level);

/* @source 0x801F43B0
 * @behavior Projects the work position into two words through 0x801F3980 and
 *           keeps the low halfword of each as the cell coordinate this step
 *           works on. It steps the first coordinate down by one and, for that
 *           pair, runs the shared 0x80155038 helper with the size 0x10, ORs
 *           the byte answered by the shared map probe 0x80166CB0 into work
 *           word +0x18 and queues a size-0x11 shared cue through 0x801A4BC0.
 *           It then steps the second coordinate up by one and repeats the
 *           pair for the cell below: 0x80155038 with the size 0x10, the
 *           probed byte into work word +0x1C and the size-0x11 cue. Finally
 *           it writes 7 into work byte 8, arms the work record published at
 *           0x80148210 with level 7 through 0x801ABBA8 and stores 2 into that
 *           record byte 7. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Evidence: the payload span 0x801F43B0-0x801F44B7 is one complete function
 * of 0x108 bytes with a single jr ra epilogue and no branch of its own, so it
 * is neither data nor a fragment; the following row is the textbin pointer run
 * at 0x801F44B8 and the span ends exactly on it. The only reference to this
 * address anywhere in the payload is one word of the data blob after that
 * pointer run, so the caller is the overlay script and its argument roles stay
 * structural. No payload under out/binaries holds these 0x108 bytes, before or
 * after masking every jal word and every materialised address word, so the
 * span has no byte-identical twin and clone is ineligible. The neighbour lifts
 * of this target supplied the spellings of 0x801F3980, 0x80166CB0 and
 * 0x801A4BC0; the work-word accumulation and the tail arm keep the shape of
 * armWorkWithScratchByte8AndSetByte7To2World00Area00013_801F2CD8.
 */
void probeLeftColumnCellCodesQueueCue11ThenArmLevel7AndSetByte7To2World01Area05213_801F43B0(
    void) {
  ProjectedWord proj_x;
  ProjectedWord proj_y;

  func_801F3980(&proj_x, &proj_y);
  ((u16*)&proj_x)[1] -= 1;
  func_80155038(((u16*)&proj_x)[1], ((u16*)&proj_y)[1], 0x10);
  FIELD_REF(u32, D_1F800044, 0x18) |=
      func_80166CB0(((u16*)&proj_x)[1], ((u16*)&proj_y)[1]) & 0xFF;
  func_801A4BC0(((u16*)&proj_x)[1], ((u16*)&proj_y)[1], 0x11);
  ((u16*)&proj_y)[1] += 1;
  func_80155038(((u16*)&proj_x)[1], ((u16*)&proj_y)[1], 0x10);
  FIELD_REF(u32, D_1F800044, 0x1C) |=
      func_80166CB0(((u16*)&proj_x)[1], ((u16*)&proj_y)[1]) & 0xFF;
  func_801A4BC0(((u16*)&proj_x)[1], ((u16*)&proj_y)[1], 0x11);
  D_1F800044[8] = 7;
  func_801ABBA8(D_80148210, 7);
  D_80148210[7] = 2;
}
