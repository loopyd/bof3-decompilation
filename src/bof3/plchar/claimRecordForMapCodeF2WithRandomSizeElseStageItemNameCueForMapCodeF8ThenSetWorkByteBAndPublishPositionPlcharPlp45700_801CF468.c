#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad work-object pointer cell of the shared main executable. The body
 * re-reads the cell before each of its two stores through it, because a store
 * through the loaded pointer may alias the cell itself.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end flag byte of the shared main executable. The world and front lifts
 * of this image describe bit 0 as a pending scenario and bit 3 as an alternate
 * front mode; this body tests bits 1 and 2 together, so the roles of those two
 * bits are not established here and the flag stays address-anchored.
 * @source 0x80143F02 @kind unknown
 */
extern u8 D_80143F02;

/* Local state byte of the shared main executable: the front-end handlers that
 * own it treat 2 as busy and every other value as free.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Twelve-byte name buffer of the shared main executable, filled from the local
 * name helper below before the selected cue is dispatched.
 * @source 0x801490D8 @kind data
 */
extern u8 D_801490D8[];
/* Terminator byte the body clears directly after the twelve bytes of the name
 * buffer above.
 * @source 0x801490E4 @kind unknown
 */
extern u8 D_801490E4[];

/* Map cell code probe of the shared EXE image, outside this payload: it answers
 * the one-byte code of the map cell at the supplied signed column and row, the
 * same helper the other lifts of this payload declare.
 */
u8 func_80166CB0(s16 col, s16 row);

/* Front-end record allocator of the shared main executable: it answers the index
 * of the next free 0x74-byte row of the front-end record table, or 0xFF when
 * every row is occupied. This body only tests the answer before claiming.
 */
u8 func_8019601C(void);

/* Record claim of this same payload: it seeds one front-end record from the
 * caller's state byte and signed cell pair and ignores the result.
 */
void claimFrontEndRecordHandler34FromPositionPairStoreStateDistanceAndClearRowByteBPlcharPlp45700_801CF660(
    u8 state, s16 x, s16 y);

/* Local name helper of the shared EXE image, outside this payload: it answers
 * the pointer to the name selected by the two arguments. This body asks for
 * entry 0 of stream 0x56 and copies exactly twelve bytes from the answer, then
 * terminates the copy, the same shape the commu00 lifts of this image use.
 */
u8* func_80165D48(s32 arg0, u8 arg1);

/* Local name test of the shared EXE image, outside this payload: it takes four
 * unsigned bytes and answers an unsigned byte. This body asks about entry 0 of
 * stream 0x56 with 1 and 0 and keeps only whether the answer is nonzero.
 */
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);

/* Cue dispatcher of the shared EXE image, outside this payload: it takes the cue
 * id in a whole word. This body passes the literal 0x106 and discards any
 * result.
 */
void func_8015DF18(u32 cue_id);

/* Shared cue dispatcher of the shared EXE image, outside this payload: the shop
 * and battle lifts pass a whole-word cue id and this body passes the small cue
 * selectors 2 and 3.
 */
void func_80150284(u32 arg0);

/* Shared size/parameter helper of the etc/game payload, outside this image
 * segment: it takes one unsigned byte. The role of the argument is not
 * established by this body, which only picks the value.
 */
void func_801B5E58(u8 arg0);

/* Shared position helper of the shared EXE image, outside this payload: it takes
 * the signed cell pair this body was given and its result is discarded, so the
 * body only has to narrow the pair.
 */
void func_801553B8(s16 x, s16 y);

/* @source 0x801CF468
 * @behavior Map cell claim of this front-end payload. The body narrows both
 *           arguments to the signed cell pair that the map probe and the record
 *           claim take and asks the shared map probe func_80166CB0 for the
 *           one-byte code of that cell. For the code 0xF2 it asks the shared
 *           allocator func_8019601C for a free front-end record and, unless that
 *           answer is 0xFF, claims one record with state 0 at the cell, rolls
 *           rand() and keeps the low four bits. A roll below 0xD stops there;
 *           from 0xD on the size starts at 5 and drops to 2 for the rolls 0xD and
 *           0xE only. When flag bits 1 and 2 of 0x80143F02 are set, a second
 *           rand() masked to two bits expands that size to twenty times its
 *           truncated byte on a zero answer. The body hands the size to
 *           func_801B5E58 and claims a second record with state 1 at the same
 *           cell, narrowing that pair again from the arguments; every path that
 *           claimed a record then stores 1 into the work byte 0xB. For the code
 *           0xF8 it claims one record with state 0 at the cell, copies the
 *           twelve-byte name that func_80165D48 answers for entry 0 of stream
 *           0x56 into 0x801490D8, clears the terminator byte 0x801490E4, and
 *           takes the cue selector 2 when func_801650B4 reports entry 0 of stream
 *           0x56 as set, after queueing cue 0x106 through func_8015DF18, and the
 *           selector 3 otherwise. It then dispatches that selector through
 *           func_80150284, marks the local state byte 0x80143BB0 busy with 2 and
 *           stores 1 into the work byte 0xB. Any other cell code returns 0 without
 *           touching state. Both claiming paths end by handing the original pair
 *           to func_801553B8 and return 1, so the caller func_801CF1E0 of this
 *           payload only runs its second attempt when no record was claimed.
 *           The 0x28-byte frame keeps $ra at 0x20 and four saved registers: the
 *           narrowed pair survives the calls in two of them and the original
 *           arguments in the other two, because the size path overwrites the
 *           narrowed second coordinate. Roles of the codes 0xF2 and 0xF8, of the
 *           size value, of the work byte 0xB and of the shared helpers are not
 *           established here, so the name keeps both codes and the address.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: both parameters stay unsigned sixteen-bit
 * and the body converts them to the signed cell pair at each call, which is why
 * the prologue and the two later call sites all narrow the same incoming register
 * through sll plus sra, while the first map probe and the first claim reuse the
 * prologue result. The size default of 5 is materialised before the roll test and
 * the 2 for the two lower rolls overrides it, and the cue selector is defined per
 * arm so the not-taken arm loads 3 into the first argument register in the delay
 * slot of the test. Measured against this payload: the 0x1F8-byte span differs
 * from the span of the same body in plp27a/00 by exactly four bytes, all of them
 * relocation or jump-displacement fields (the local record claim at 0x801CF4D8
 * and 0x801CF590, and the intra-payload jump at 0x801CF578), so the sibling is
 * shape evidence for this statement order and not a clone source. The sibling
 * caller func_801CF1E0 of this payload declares the pair as signed sixteen-bit in
 * its own file; both spellings carry the same low half, so no shared header is
 * involved.
 */
u8 claimRecordForMapCodeF2WithRandomSizeElseStageItemNameCueForMapCodeF8ThenSetWorkByteBAndPublishPositionPlcharPlp45700_801CF468(
    u16 x, u16 y) {
  u8* name;
  u8 code;
  u8 index;
  u32 size;
  u32 roll;
  u32 mode;
  s16 col;
  s16 row;
  s32 i;

  col = x;
  row = y;
  code = func_80166CB0(col, row);
  if (code == 0xF2) {
    index = func_8019601C();
    if (index != 0xFF) {
      claimFrontEndRecordHandler34FromPositionPairStoreStateDistanceAndClearRowByteBPlcharPlp45700_801CF660(
          0, col, row);
      roll = rand() & 0xF;
      if (roll >= 0xD) {
        size = 5;
        if (roll < 0xF) {
          size = 2;
        }
        if ((D_80143F02 & 6) != 0) {
          if ((rand() & 3) == 0) {
            size = (size & 0xFF) * 20;
          }
        }
        func_801B5E58(size);
        claimFrontEndRecordHandler34FromPositionPairStoreStateDistanceAndClearRowByteBPlcharPlp45700_801CF660(
            1, x, y);
      }
      D_1F800044[0xB] = 1;
    }
    func_801553B8(x, y);
    return 1;
  } else if (code == 0xF8) {
    claimFrontEndRecordHandler34FromPositionPairStoreStateDistanceAndClearRowByteBPlcharPlp45700_801CF660(
        0, col, row);
    name = func_80165D48(0, 0x56);
    for (i = 0; i < 12; i++) {
      D_801490D8[i] = name[i];
    }
    D_801490E4[0] = 0;
    if (func_801650B4(0, 0x56, 1, 0) != 0) {
      func_8015DF18(0x106);
      mode = 2;
    } else {
      mode = 3;
    }
    func_80150284(mode);
    D_80143BB0 = 2;
    D_1F800044[0xB] = 1;
    func_801553B8(x, y);
    return 1;
  } else {
    return 0;
  }
}
