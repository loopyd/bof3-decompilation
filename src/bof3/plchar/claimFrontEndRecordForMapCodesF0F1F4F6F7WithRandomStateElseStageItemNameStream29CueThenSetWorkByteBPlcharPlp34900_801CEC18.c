#include "bof3/bof3.h"

/* PsyQ libc random source of the shared EXE image, outside this payload. The
 * body keeps only its low three and low four bits, and the two rolls select the
 * extra claim and the follow-up arm.
 */
extern int rand(void);

/* Scratchpad work-object pointer cell of the shared main executable. The body
 * re-reads the cell once per store through it, because a store through the
 * loaded pointer may alias the cell itself, so the source keeps the named cell
 * and evaluates it at each store.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end phase byte of the shared main executable: the handlers that own it
 * treat 2 as busy. This body stores 2 on the claim arm, once on the name arm and
 * twice on the state-2 arm.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Front-end record published at 0x80146250 of the shared main executable. The
 * body reads its byte 0x79 and passes it to func_8016813C; the role of that byte
 * is not established here.
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
 * the code of the map cell at the supplied signed column and row, the same
 * helper the other lifts of this payload declare. The body keeps the answer as a
 * whole word and never narrows it on the way in, so the probe is declared word
 * returning here.
 */
extern s32 func_80166CB0(s16 col, s16 row);

/* Local name helper of the shared EXE image, outside this payload: it answers
 * the pointer to the name selected by the two arguments. This body asks for
 * entry 0 of stream 0x29 and copies exactly twelve bytes from the answer.
 */
extern u8* func_80165D48(s32 arg0, u8 arg1);

/* Local name test of the shared EXE image, outside this payload: it takes four
 * unsigned bytes and answers an unsigned byte. This body asks about entry 0 of
 * stream 0x29 with 1 and 0 and keeps only whether the answer is nonzero.
 */
extern u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);

/* Cue dispatcher of the shared EXE image, outside this payload: it takes the cue
 * id in a whole word. This body passes the literals 0x10B and 0x106 and discards
 * any result.
 */
extern void func_8015DF18(u32 cue);

/* Shared cue dispatcher of the shared EXE image, outside this payload: the shop
 * and battle lifts pass a whole-word cue id and this body passes the small cue
 * selectors 2, 3 and 0xD9.
 */
extern void func_80150284(u32 arg0);

/* Shared helper of the shared EXE image, outside this payload. This body passes
 * a literal zero and discards any result, so neither the arity nor the role of
 * that argument is established.
 */
extern void func_801C308C(s32 arg0);

/* Shared helper of the shared EXE image, outside this payload: it takes the
 * literal 1 and the byte 0x79 of the record published at 0x80146250, and this
 * body discards any result.
 */
extern void func_8016813C(s32 arg0, u8 arg1);

/* Front-end record claim of this same payload, carried by the exact lift of its
 * own row at 0x801CEE24: it takes the entry state byte this body passes plus the
 * signed cell pair and seeds one front-end record through the shared allocator
 * func_8019601C, answering nothing. All five in-image call sites are inside this
 * body, and every one of them also fills $a3 with the masked map cell code; that
 * claim body does not read it, so the prototype here carries the fourth argument
 * the original call shape needs while the row that owns the body keeps the three
 * it uses.
 */
extern void claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp34900_801CEE24(
    u8 state, s16 x, s16 y, u8 masked);

/* @source 0x801CEC18
 * @behavior Map cell claim of this front-end payload, run by the projected-cell
 *           step 0x801CE7B8 from its three plain `jal` sites 0x801CE950,
 *           0x801CE978 and 0x801CE9A0, each time with the high half of one
 *           projected work word pair and then again one column and one row
 *           further whenever the previous answer was zero. The body narrows both
 *           arguments to the signed cell pair that the map probe and the record
 *           claim take and asks the shared map probe func_80166CB0 for the code
 *           of that cell. When that code is 0xF0, 0xF1 or 0xF4 it claims one
 *           front-end record with entry state 0 at the cell, rolls rand() and,
 *           when the low three bits read six or more, claims a second record with
 *           entry state 4 at the same cell, then queues cue 0x10B and answers
 *           one. When the code is instead 0xF6 or 0xF7 it claims the state 0
 *           record at the cell and queues cue 0x10B as well, then rolls rand()
 *           masked to four bits: a roll below 7 claims a state 3 record at the
 *           cell, copies the twelve-byte name that func_80165D48 answers for
 *           entry 0 of stream 0x29 into 0x801490D8, clears the terminator byte
 *           0x801490E4, and takes the cue selector 2 when func_801650B4 reports
 *           entry 0 of stream 0x29 as set, after queueing cue 0x106, and the
 *           selector 3 otherwise, then dispatches that selector through
 *           func_80150284 and stores 2 into the work byte 0xB; a roll of 7 to 11
 *           writes nothing there; a roll of 12 or more claims a state 2 record at
 *           the cell, runs the shared no-argument helper func_801C308C, hands the
 *           literal 1 and byte 0x79 of the record published at 0x80146250 to
 *           func_8016813C, dispatches cue 0xD9 through func_80150284, marks the
 *           local state byte 0x80143BB0 busy with 2 and stores 1 into the work
 *           byte 0xB. Those three rolls all end by marking 0x80143BB0 busy with 2
 *           and answering one, while the state 2 roll marks it a second time
 *           through that shared tail. Any other cell code answers 0 without
 *           touching state. The body takes the cell pair as two unsigned
 *           sixteen-bit arguments, because the prologue narrows both through the
 *           shift pair before the first call, and answers one or zero as an
 *           unsigned byte. Roles of the codes 0xF0, 0xF1, 0xF4, 0xF6 and 0xF7, of
 *           the roll bounds, of the entry state values 0, 2, 3 and 4, of the work
 *           byte 0xB and of the shared helpers are not established here, so the
 *           name keeps all five codes and the address.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the 0x20C-byte span at 0x801CEC18 was
 * compared word for word against the exact plp056 lift of the twin body at
 * 0x801CFF4C and the two 131-word spans differ in exactly eight words, the five
 * `jal` words of the local claim 0x801CEE24 and the three intra-body jumps, so
 * the statement order, operand forms and declaration style were taken from that
 * exact lift with this function's own name. The map code is a whole-word value,
 * because the original moves the probe answer into its register without a mask
 * and compares it as an unsigned value, so the probe is declared word returning
 * and the byte view appears only in the two explicit narrowings; with a
 * byte-typed local the prologue narrows the answer and the first range test turns
 * into a signed compare. The 0xF4 comparison narrows through the byte cast while
 * the five claim arguments mask the same value, because the original computes
 * that byte twice, once before the comparison and again in the join block the
 * 0xF0 and 0xF1 codes jump to, and then keeps the second copy in a saved register
 * for the remaining four call sites. The range tests themselves are the
 * `(value + bias) & 0xFF < 2` form the original uses to fold each pair of codes
 * into one comparison: the byte mask makes them unsigned, so both are `sltiu`.
 */
u8 claimFrontEndRecordForMapCodesF0F1F4F6F7WithRandomStateElseStageItemNameStream29CueThenSetWorkByteBPlcharPlp34900_801CEC18(
    u16 x, u16 y) {
  u8* name;
  u32 code;
  u32 roll;
  u32 mode;
  s16 col;
  s16 row;
  s32 i;

  col = x;
  row = y;
  code = func_80166CB0(col, row);
  if (((code + 0x10) & 0xFF) < 2 || (u8)code == 0xF4) {
    claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp34900_801CEE24(
        0, col, row, code & 0xFF);
    if ((rand() & 7) >= 6) {
      claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp34900_801CEE24(
          4, col, row, code & 0xFF);
    }
    func_8015DF18(0x10B);
    return 1;
  } else if (((code + 0xA) & 0xFF) < 2) {
    claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp34900_801CEE24(
        0, col, row, code & 0xFF);
    func_8015DF18(0x10B);
    roll = rand() & 0xF;
    if (roll < 7) {
      claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp34900_801CEE24(
          3, col, row, code & 0xFF);
      name = func_80165D48(0, 0x29);
      for (i = 0; i < 12; i++) {
        D_801490D8[i] = name[i];
      }
      D_801490E4[0] = 0;
      if ((func_801650B4(0, 0x29, 1, 0) & 0xFF) != 0) {
        func_8015DF18(0x106);
        mode = 2;
      } else {
        mode = 3;
      }
      func_80150284(mode);
      D_1F800044[0xB] = 2;
    } else if (roll >= 0xC) {
      claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp34900_801CEE24(
          2, col, row, code & 0xFF);
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
