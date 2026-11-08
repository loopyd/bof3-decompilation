#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad work-object pointer cell of the shared main executable. The body
 * re-reads the cell before each store through it, because a store through the
 * loaded pointer may alias the cell itself.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Local state byte of the shared main executable: the front-end handlers that
 * own it treat 2 as busy and every other value as free. This body marks it busy
 * on every path that claimed a row.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Front-end record pointer cell of the shared main executable. This body reads
 * byte 0x79 of the record published here as the argument of the shared helper
 * func_8016813C; the role of that byte is not established in this overlay, so it
 * stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

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

/* Front-end record claim of this same payload, already carried by the exact lift
 * of its own row in this target: it takes the entry state byte this body passes
 * plus the signed cell pair and seeds one front-end record through the shared
 * allocator func_8019601C, and it answers nothing. The claim body never reads a
 * fourth argument, yet every call site of this body sets $a3 to the probed cell
 * code, so the caller-side declaration keeps that fourth word.
 */
void claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp02600_801D0354(
    u8 state, s16 x, s16 y, u8 code);

/* Local name helper of the shared EXE image, outside this payload: it answers
 * the pointer to the name selected by the two arguments. This body asks for
 * entry 0 of stream 0x29 and copies exactly twelve bytes from the answer, then
 * terminates the copy.
 */
u8* func_80165D48(s32 arg0, u8 arg1);

/* Local name test of the shared EXE image, outside this payload: it takes four
 * unsigned bytes and answers an unsigned byte. This body asks about entry 0 of
 * stream 0x29 with 1 and 0 and keeps only whether the answer is nonzero.
 */
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);

/* Cue dispatcher of the shared EXE image, outside this payload: it takes the cue
 * id in a whole word. This body passes the literals 0x10B and 0x106 and discards
 * any result.
 */
void func_8015DF18(u32 cue_id);

/* Shared cue dispatcher of the shared EXE image, outside this payload: the shop
 * and battle lifts pass a whole-word cue id and this body passes the small cue
 * selectors 2, 3 and 0xD9.
 */
void func_80150284(u32 arg0);

/* Shared front-end helper of the etc/game payload, outside this payload: its own
 * asm row narrows the first argument to a byte, and this body passes 0.
 */
void func_801C308C(u8 arg0);

/* Shared helper of the etc/game payload family, outside this payload: this body
 * passes 1 and byte 0x79 of the record published at 0x80146250, and discards any
 * result.
 */
void func_8016813C(s32 arg0, u8 arg1);

/* @source 0x801D0148
 * @behavior Map cell claim step of this front-end payload. The body narrows both
 *           arguments to the signed cell pair the map probe and the record claim
 *           take and asks the shared map probe func_80166CB0 for the one-byte
 *           code of that cell. For the codes 0xF0 and 0xF1 (spelled
 *           (u8)(code + 0x10) below 2) and 0xF4 it claims one front-end record
 *           with state 0 at the cell and, when rand() masked to three bits reads
 *           6 or 7, a second one with state 4 at the same cell, then queues cue
 *           0x10B and answers 1. For the codes 0xF6 and 0xF7 (spelled
 *           (u8)(code + 0xA) below 2) it claims one record with state 0, queues
 *           cue 0x10B and rolls rand() masked to four bits: a roll below 7 claims
 *           a record with state 3, copies the twelve-byte name that func_80165D48
 *           answers for entry 0 of stream 0x29 into 0x801490D8, clears the
 *           terminator byte 0x801490E4, takes the cue selector 2 when
 *           func_801650B4 reports entry 0 of stream 0x29 as set and 3 otherwise,
 *           after queueing cue 0x106 in the set case, dispatches that selector
 *           through func_80150284 and stores 2 into the work byte 0xB; a roll of
 *           0xC or more claims a record with state 2, runs func_801C308C with 0,
 *           passes 1 and byte 0x79 of the record at 0x80146250 to func_8016813C,
 *           dispatches cue 0xD9 through func_80150284, marks the local state byte
 *           busy with 2 and stores 1 into the work byte 0xB; the rolls 7 to 0xB
 *           claim nothing further. Every path that reached the roll marks the
 *           local state byte 0x80143BB0 busy with 2 once more after the work byte
 *           store and answers 1, and any other cell code answers 0 without
 *           touching state, which is why the 0x801CFCD8 step retries the
 *           neighbouring cells only on a zero answer. The 0x20-byte frame keeps
 *           $ra at 0x1C and $s2..$s0 at 0x18..0x10 across the calls: the narrowed
 *           pair survives in two of them and the masked cell code in the third.
 *           Roles of the work byte 0xB, of the local state byte and of the shared
 *           helpers are not established here, so the name keeps both code groups
 *           and the address.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the probe answer is kept in a byte and
 * masked at each use, so the two code-group tests are an add followed by a byte
 * mask and an unsigned compare, and the 0xF4 test masks the same value again for
 * the fourth argument of every claim call. Both claim calls of the first group
 * pass that masked byte in $a3 although the claim body of this payload never
 * reads a fourth argument, so the caller-side declaration keeps the extra word.
 * The constant 0 of the first group claim and the add of the second code-group
 * test each appear twice, because the reorg pass duplicated them into a branch
 * delay slot. The local state byte is stored twice on the roll 0xC path: once in
 * that arm, as the original source order has it, and once in the shared tail,
 * which the store through the re-read pointer cell can alias. The second group
 * is written as a positive test whose false arm returns 0 at the end of the
 * function, which is why the 0 answer sits after the shared tail and not inline.
 */
u8 claimFrontEndRowForMapCodesF0F1F4OrRandomRowForF6F7ThenStageNameOrSetWorkByteBPlcharPlp02600_801D0148(
    u16 x, u16 y) {
  u8* name;
  u8 code;
  u32 roll;
  u32 mode;
  s16 col;
  s16 row;
  s32 i;

  col = x;
  row = y;
  code = func_80166CB0(col, row);
  if ((u8)(code + 0x10) < 2 || (u8)code == 0xF4) {
    claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp02600_801D0354(
        0, col, row, (u8)code);
    if ((rand() & 7) >= 6) {
      claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp02600_801D0354(
          4, col, row, (u8)code);
    }
    func_8015DF18(0x10B);
    return 1;
  }
  if ((u8)(code + 0xA) < 2) {
    claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp02600_801D0354(
        0, col, row, (u8)code);
    func_8015DF18(0x10B);
    roll = rand() & 0xF;
    if (roll < 7) {
      claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp02600_801D0354(
          3, col, row, (u8)code);
      name = func_80165D48(0, 0x29);
      for (i = 0; i < 12; i++) {
        D_801490D8[i] = name[i];
      }
      D_801490E4[0] = 0;
      if (func_801650B4(0, 0x29, 1, 0) != 0) {
        func_8015DF18(0x106);
        mode = 2;
      } else {
        mode = 3;
      }
      func_80150284(mode);
      D_1F800044[0xB] = 2;
    } else if (roll >= 0xC) {
      claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp02600_801D0354(
          2, col, row, (u8)code);
      func_801C308C(0);
      func_8016813C(1, D_80146250[0x79]);
      func_80150284(0xD9);
      D_80143BB0 = 2;
      D_1F800044[0xB] = 1;
    }
    D_80143BB0 = 2;
    return 1;
  }
  return 0;
}
