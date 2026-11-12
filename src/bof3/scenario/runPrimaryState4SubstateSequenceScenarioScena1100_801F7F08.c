#include "bof3/bof3.h"

/* PsyQ libc random source of the shared EXE image, outside this payload. The
 * body keeps only the low four bits of one roll and indexes the roll table
 * with them. */
extern int rand(void);

/* PsyQ libc bounded copy of the shared EXE image, outside this payload: it
 * copies the twelve-byte name the local name helper answers into the shared
 * name buffer below. The body discards the answer. */
extern void strncpy(u8 *dst, const u8 *src, u32 count);

/* Shared front-end mode byte of the shared EXE image: every claim arm of this
 * payload treats 2 as busy and refuses to act. */
extern u8 D_80143BB0;

/* Front-end record table of the shared EXE image, declared once per record
 * field address the way the sibling plchar lifts of the same table spell it:
 * the address of each byte and word the body stores carries its own symbol, so
 * the table base never becomes a hoisted register and every store re-forms its
 * own address from the scaled work index. The generated
 * undefined_syms_auto.txt rows bind these names. */
extern u8 D_80143FC8[];
extern u8 D_80143FC9[];
extern u8 D_80143FCD[];
extern u8 D_80143FFC[];
extern u8 D_80144000[];
extern u8 D_80144004[];

/* Front-end work-record run of the shared EXE image, published by the claim
 * arm as the work record. */
extern u8 D_80145E90[];

/* Selected-battler class byte of the shared EXE image. This body uses it as a
 * mode gate in two arms and does not write it. */
extern u8 D_80145F09;

/* Published work-record pointer cell of the shared EXE image. */
extern u8 *D_80146250;

/* Shared scenario progress byte and shared state bytes of the main executable;
 * the clearing tail zeroes all but the progress byte, which the arm zeroes
 * too. */
extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

/* Twelve-byte name buffer of the shared EXE image and the terminator byte the
 * body clears directly after it. */
extern u8 D_801490D8[];
extern u8 D_801490E4;

/* Scratchpad byte 0 of the shared main executable. Case 0 stores the claimed
 * work index there and re-reads it for every record store, because a store
 * through the record table may alias the slot itself. */
extern u8 D_1F800000[];

/* Scratchpad work-record pointer cell at 0x1F800044, which this payload's
 * dispatchers read as the work record.
 * @source 0x1F800044 @kind unknown
 */
extern u8 *D_1F800044;

/* Sixteen-byte roll table of this target at 0x801FAECC, inside the leading
 * table data run that starts at 0x801FAEC0. Indexing it with the low four bits
 * of a random roll answers the handler id the claim arm stores in the record
 * byte 1 that dispatchWorkByte1HandlerThenPublishMode2ScenarioScena1100_801F70B4
 * later reads, so the table maps a roll onto one of the ids 0, 2, 4 and 5. */
extern u8 D_801FAECC[]; /* @source 0x801FAECC @kind data */

/* Shared front-end record allocator of the shared EXE image: it answers the
 * work index of the record it just seeded, which is the byte this body
 * publishes in scratchpad byte 0. */
extern u8 func_8019601C(void);

/* Shared no-argument helper of the shared EXE image. This body passes a
 * literal zero and discards any result. */
extern void func_801C308C(s32 arg0);

/* Shared helper of the shared EXE image: this body passes the literal 1 and
 * the literal 6 and discards any result. */
extern void func_8016813C(s32 arg0, u8 arg1);

/* Shared cue dispatcher of the shared EXE image: this body passes the cue
 * selectors 0xD9 and 3 and discards any result. */
extern void func_80150284(u32 arg0);

/* Shared front-end action starter of the shared EXE image: this body passes
 * the action ids 0x42 and 0 and discards any result. */
extern void func_80150224(u32 arg0);

/* Shared entry test of the shared EXE image: it takes four unsigned bytes and
 * answers an unsigned byte. Case 5 asks about entry 0 of stream 0x23 with 9
 * and 0 and keeps only whether the answer is nonzero; case 0x15 asks about
 * entry 0 of stream 0x56 with 1 and 0 and discards the answer. */
extern u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);

/* Local name helper of the shared EXE image: it answers the pointer to the
 * name selected by the two arguments. Case 5 asks for entry 0 of stream 0x23
 * and copies exactly twelve bytes from the answer. */
extern u8 *func_80165D48(s32 arg0, u8 arg1);

/* Shared sound-cue dispatcher of the main executable, bound by this target's
 * own map. Both case 6 and case 0x14 dispatch cue 0x106. */
extern void dispatchSoundCue(u32 cue_id);

/* Shared overlay release helper of the shared EXE image, run first in the
 * clearing tail. */
extern void func_8015C058(void);

/* Front-end record claim of this same payload, bound by its own exact row at
 * 0x801F82D8: it takes the work index scratchpad byte 0 holds, publishes that
 * record as the work record and resets it. */
extern void
claimFrontEndRecordPublishWorkPointerAndResetStateScenarioScena1100_801F82D8(
    u8 index);

/* @source 0x801F7F08
 * @behavior Word 4 (the word stored at 0x801FAF0C) of the ten-entry
 * overlay-local second-level handler run at 0x801FAEFC that the exact
 * dispatcher dispatchSecondaryStateScenarioScena1100_801F7748 (0x801F7748)
 * indexes with the shared primary state byte D_80146874, so a primary state
 * reading 4 selects it; no jal or j of the payload names 0x801F7F08, and that
 * little-endian table word is its only reference. It switches on the shared
 * secondary sub-state byte D_80146875 (read with lbu) through the
 * fifty-one-word absolute jump table jtbl_801F6DA4_main at 0x801F6DA4 in this
 * payload leading table data, bounded by `sltiu` against 0x33; sub-states 3, 4,
 * 7 through 0x13 and 0x16 through 0x31 are the table words that point at the
 * body exit, so those values return without touching any byte.
 *
 * Sub-state 0 first gates on the shared class byte D_80145F09: while it is not
 * 6 the arm stores 0x32 in the sub-state byte and stops. Otherwise it asks the
 * shared allocator func_8019601C (0x8019601C) for a work index, publishes it
 * in scratchpad byte 0 and stops if the masked index reads 0xFF. It then seeds
 * that front-end record through the shared table D_80143FC8 (0x80143FC8) at
 * the 0x74-byte stride the multiply expands to: byte 0 = 1, byte 5 = 0x34,
 * word 0x34 = 0x250000, word 0x38 = 0xB0000 and word 0x3C = 0x5000000, each
 * store re-reading the index from scratchpad byte 0. One rand roll masked to
 * four bits then indexes the sixteen-byte roll table D_801FAECC (0x801FAECC)
 * for the handler id it stores in record byte 1, and the same table read again
 * picks the exit: id 0 stores 0x32 in the sub-state byte, id 1 leaves the
 * sub-state byte alone, id 2 advances it to 1, id 4 stores 0x32, and id 5 runs
 * the claim at 0x801F82D8 with the scratchpad index and advances the sub-state
 * byte to 5.
 *
 * Sub-state 1 publishes the work-record run D_80145E90 (0x80145E90) into the
 * scratchpad pointer cell 0x1F800044 and into D_80146250 (0x80146250), runs
 * func_801C308C with 0, runs func_8016813C with 1 and 6, then dispatches cue
 * 0xD9 through func_80150284. Sub-state 2, while the mode byte D_80143BB0
 * (0x80143BB0) is not 2, stores 0x32 in the sub-state byte. Sub-state 5 asks
 * func_801650B4 about entry 0 of stream 0x23: a non-zero answer queues action
 * 0x42 through func_80150224 (0x80150224), stores 2 in the mode byte and
 * advances the sub-state byte to 6, while a zero answer copies the twelve-byte
 * name func_80165D48 (0x80165D48) answers for entry 0 of stream 0x23 through
 * strncpy into the shared name buffer D_801490D8 (0x801490D8), clears the
 * terminator byte D_801490E4 (0x801490E4), dispatches cue 3 through
 * func_80150284, stores 2 in the mode byte and advances the sub-state byte to
 * 2. Both halves of sub-state 1 and 5 end in one shared cue-dispatch tail that
 * stores 2 in the mode byte and in the sub-state byte.
 *
 * Sub-state 6, while the mode byte is not 2, dispatches cue 0x106 and stores
 * 0x32 in the sub-state byte. Sub-state 0x14, once the class byte D_80145F09
 * reads 7, dispatches cue 0x106, queues action 0 through func_80150224, stores
 * 2 in the mode byte and advances the sub-state byte to 0x15; before that it
 * stores 0x32. Sub-state 0x15, while the mode byte is not 2, asks func_801650B4
 * about entry 0 of stream 0x56 with 1 and 0, discards the answer and stores
 * 0x32 in the sub-state byte. Sub-state 0x32 is the clearing tail: it runs the
 * shared overlay release helper func_8015C058 (0x8015C058) and then clears the
 * shared progress byte g_ScenarioProgress (0x80146864), the shared bytes
 * 0x80146865, 0x80146866 and 0x80146867, the sub-state byte D_80146875 and the
 * primary state byte D_80146874 in that order.
 *
 * It takes no arguments and returns nothing; its 0x20-byte frame keeps $ra,
 * $s1 and $s0. $s0 holds the scratchpad base address taken once per claim arm
 * and $s1 holds the constant 1, which the record byte 0 store and the id-2
 * sub-state store share across the rand call. Every shared byte, table and
 * helper above is addressed from the main executable, so the span has no
 * target-local copy: a scan of the 791 extracted payloads under out/binaries
 * for the three constant loads `lui $v1,0x25`, `lui $v1,0x0B` and
 * `lui $v1,0x500` inside one 0x90-byte window finds this payload only, at
 * 0x801F7FE4, 0x801F8010 and 0x801F803C, so the clone lever is declined and
 * every literal here was written from this span's own bytes. The nearest shape donors are the exact
 * neighbours at 0x801F7CC8 and 0x801FA3A4 of the same handler run, which
 * supply the sub-state switch over an absolute jump table, the mode-byte
 * guarded arm and the clearing tail, and the plchar lift of 0x801CF224, which
 * supplies the shared name-copy, cue-dispatch and action-queue statement order
 * this body uses in sub-states 1 and 5.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState4SubstateSequenceScenarioScena1100_801F7F08(void) {
  u8 *slot;
  u8 index;
  u8 *name;
  u32 roll;

  switch (D_80146875) {
  case 0:
    if (D_80145F09 != 6u) {
      D_80146875 = 0x32u;
      break;
    }
    index = func_8019601C();
    slot = D_1F800000;
    slot[0] = index;
    if (index == 0xFF) {
      break;
    }
    D_80143FC8[(u32)index * 0x74u] = 1;
    D_80143FCD[(u32)slot[0] * 0x74u] = 0x34;
    *(u32 *)(D_80143FFC + (u32)slot[0] * 0x74u) = 0x250000;
    *(u32 *)(D_80144000 + (u32)slot[0] * 0x74u) = 0xB0000;
    *(u32 *)(D_80144004 + (u32)slot[0] * 0x74u) = 0x5000000;
    roll = rand() & 0xF;
    D_80143FC9[(u32)slot[0] * 0x74u] = D_801FAECC[roll];
    switch (D_801FAECC[roll]) {
    case 0:
      D_80146875 = 0x32u;
      break;
    case 2:
      D_80146875 = 1u;
      break;
    case 4:
      D_80146875 = 0x32u;
      break;
    case 5:
      claimFrontEndRecordPublishWorkPointerAndResetStateScenarioScena1100_801F82D8(
          slot[0]);
      D_80146875 = 5u;
      break;
    }
    break;
  case 1:
    D_1F800044 = D_80145E90;
    D_80146250 = D_80145E90;
    func_801C308C(0);
    func_8016813C(1, 6);
    func_80150284(0xD9);
    D_80143BB0 = 2u;
    D_80146875 = 2u;
    break;
  case 2:
    if (D_80143BB0 == 2u) {
      break;
    }
    D_80146875 = 0x32u;
    break;
  case 5:
    if ((func_801650B4(0, 0x23, 9, 0) & 0xFF) == 0) {
      name = func_80165D48(0, 0x23);
      strncpy(D_801490D8, name, 12);
      D_801490E4 = 0;
      func_80150284(3);
      D_80143BB0 = 2u;
      D_80146875 = 2u;
    } else {
      func_80150224(0x42u);
      D_80143BB0 = 2u;
      D_80146875 = 6u;
    }
    break;
  case 6:
    if (D_80143BB0 == 2u) {
      break;
    }
    dispatchSoundCue(0x106);
    D_80146875 = 0x32u;
    break;
  case 0x14:
    if (D_80145F09 != 7u) {
      D_80146875 = 0x32u;
      break;
    }
    dispatchSoundCue(0x106);
    func_80150224(0);
    D_80143BB0 = 2u;
    D_80146875 = 0x15u;
    break;
  case 0x15:
    if (D_80143BB0 == 2u) {
      break;
    }
    func_801650B4(0, 0x56, 1, 0);
    D_80146875 = 0x32u;
    break;
  case 0x32:
    func_8015C058();
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  }
}
