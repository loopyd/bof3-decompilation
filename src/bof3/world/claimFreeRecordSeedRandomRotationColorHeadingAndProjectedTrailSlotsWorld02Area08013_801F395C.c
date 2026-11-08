#include "bof3/bof3.h"

extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_801F44A4; /* @source 0x801F44A4 @kind unknown */
extern u8 D_801F3DC4[]; /* @source 0x801F3DC4 @kind table */
extern int rand(void);
u8* findFreeRecordAt800E486CWorld02Area08013_801F3B98(void);

/* The original copies the colour word and the two projected trail pairs with
 * `lwl`/`lwr` + `swl`/`swr` word pairs, which is the code this compiler emits
 * for a word-sized block move whose type has alignment 1: every copied value
 * below is a byte aggregate for that reason, not a plain `u32` cell. */
typedef struct RecordWordBytes {
  u8 bytes[4];
} RecordWordBytes;
s16 func_8015477C(s32 arg0, s32 arg1);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801F395C
 * @behavior Spawn step of the six-word handler run based at 0x801F3DAC: its
 *           caller, the handler at 0x801F2DE0, runs it after the table walk at
 *           0x801F3870 and before it decrements the work countdown byte. It
 *           claims the first free record of the 0x800E486C table through
 *           findFreeRecordAt800E486CWorld02Area08013_801F3B98, publishes that
 *           record as the shared cursor D_801F44A4 and returns without seeding
 *           anything when the shared frame counter D_80143E6C is not on a
 *           multiple of four or when no record was free. Otherwise it copies the
 *           work record position words at +0x34 and +0x38 from the scratchpad
 *           cell 0x1F800044 into record +0x18 and +0x1C and the word at +0x3C,
 *           biased by 0x06000000, into +0x20; takes a random twelve-bit angle
 *           and a random spread of -0x100..0xFF, rotates the work position by
 *           that angle through the shared 12-bit rotation pair
 *           func_801783C8/func_801782FC scaled by 256 into +0x28 and +0x2C,
 *           stores the heading func_8015477C of that rotated point scaled by
 *           0x10000 into +0x30, and derives the velocity words at +0x8 and +0xC
 *           from the second angle (the first angle plus the spread plus 0x800)
 *           scaled by 16. It clears +0x10, copies one word of the eight-entry
 *           word table at 0x801F3DC4 picked by a random low three-bit index into
 *           the colour word at +4, marks the record claimed with byte +0 = 1 and
 *           typed with byte +1 = 0x20, fills the 0x20-byte projection scratch
 *           through func_801AFE18, projects the record position (+0x18) into
 *           +0x40 and the rotated point (+0x28) into +0x50 through the shared
 *           projector func_801AFF04, and then copies each of those two projected
 *           pairs into the next three words of its own slot group so the four
 *           entries at +0x40 and at +0x50 all start on the fresh pair. Takes no
 *           arguments and returns nothing; its 0x50-byte frame holds $ra at
 *           0x48, $s1 at 0x44, $s0 at 0x40 and a 0x30-byte scratch area at 0x10
 *           whose first 0x20 bytes are the ones the projector consumes.
 *           Every literal and cell above comes from these original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreeRecordSeedRandomRotationColorHeadingAndProjectedTrailSlotsWorld02Area08013_801F395C(
    void) {
  u8 scratch[0x30];
  u16 angle;
  s16 spread;
  s32 turn;
  u8 i;

  D_801F44A4 = findFreeRecordAt800E486CWorld02Area08013_801F3B98();
  if ((D_80143E6C & 3) != 0) {
    return;
  }
  if (D_801F44A4 == 0) {
    return;
  }
  *(s32*)(D_801F44A4 + 0x18) = *(s32*)(D_1F800044 + 0x34);
  *(s32*)(D_801F44A4 + 0x1C) = *(s32*)(D_1F800044 + 0x38);
  *(s32*)(D_801F44A4 + 0x20) = *(s32*)(D_1F800044 + 0x3C) + 0x06000000;
  angle = rand() & 0xFFF;
  spread = (rand() & 0x1FF) - 0x100;
  *(s32*)(D_801F44A4 + 0x28) =
      *(s32*)(D_1F800044 + 0x34) + (func_801783C8(angle) << 8);
  *(s32*)(D_801F44A4 + 0x2C) =
      *(s32*)(D_1F800044 + 0x38) + (func_801782FC(angle) << 8);
  *(s32*)(D_801F44A4 + 0x30) =
      func_8015477C(*(s32*)(D_801F44A4 + 0x28), *(s32*)(D_801F44A4 + 0x2C))
      << 16;
  turn = angle + spread + 0x800;
  *(s32*)(D_801F44A4 + 0x8) = func_801783C8(turn) << 4;
  *(s32*)(D_801F44A4 + 0xC) = func_801782FC(turn) << 4;
  *(s32*)(D_801F44A4 + 0x10) = 0;
  *(RecordWordBytes*)(D_801F44A4 + 4) =
      *(RecordWordBytes*)(D_801F3DC4 + ((rand() & 7) << 2));
  D_801F44A4[0] = 1;
  D_801F44A4[1] = 0x20;
  func_801AFE18(scratch);
  func_801AFF04(D_801F44A4 + 0x18, D_801F44A4 + 0x40);
  func_801AFF04(D_801F44A4 + 0x28, D_801F44A4 + 0x50);
  for (i = 1; i < 4; i++) {
    *(RecordWordBytes*)(D_801F44A4 + 0x40 + (i << 2)) =
        *(RecordWordBytes*)(D_801F44A4 + 0x40);
    *(RecordWordBytes*)(D_801F44A4 + 0x50 + (i << 2)) =
        *(RecordWordBytes*)(D_801F44A4 + 0x50);
  }
}
