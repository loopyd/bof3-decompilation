#include "bof3/bof3.h"

/* Work record published through the scratchpad pointer cell at 0x1F800044. The
 * original re-materialises the cell for every access group - the byte-8 store,
 * the view arm, the arming arm and the closing +0x14/+0x3E pair each open their
 * own `lui 0x1F80` plus folded `lw 68` pair - so the cell stays a named pointer
 * symbol that every access re-reads instead of a constant the allocator could
 * hoist into a callee-saved register.
 * @source 0x1F800044 @kind data
 */
extern u8* workRecordCursor1F800044;

/* Pointer cell of the shared record this step arms. The original re-reads it at
 * every access site - the byte-7 store, the byte-7 read-back test, the arming
 * call's record argument and each arm's flags update - which is the cell reload
 * a store through the loaded pointer and a call may force; this body only
 * reaches its bytes 0 and 7, so the byte view is the one declared. The
 * neighbouring world lifts read the same cell.
 * @source 0x80148210 @kind unknown
 */
extern u8* D_80148210;

/* Shared small mode byte of the main executable. The neighbour at 0x801F4B40
 * stores 0 into it and the neighbour at 0x801F4864 steps it while it reads
 * below 8; this body scales it by four into byte 7 of the published record.
 * @source 0x8014686B @kind unknown
 */
extern u8 D_8014686B;

/* Shared facing halfword, cleared together with the work record's word at
 * +0x14, the same pair the reset step at 0x801F564C clears.
 * @source 0x80149330 @kind data
 */
extern u16 D_80149330;

/* Shared main-executable status helper: its callers compare the returned word
 * against the small constants 1 and 2, so it is a small status code.
 * @source 0x8015C148
 */
extern s32 func_8015C148(void);

/* Shared main-executable helper that arms the published record for the level
 * byte passed as its second argument.
 * @source 0x801ABBA8
 */
extern void func_801ABBA8(u8* record, u8 level);

/* Shared main-executable view routine; its byte argument selects the view mode
 * it scales the view step with.
 * @source 0x801C5DFC
 */
extern void func_801C5DFC(u8 mode);

/* @source 0x801F49C8
 * @behavior Overlay step handler. It loads the scratch work record published at
 *   the scratchpad pointer cell 0x1F800044, stores the level 1 into its byte
 *   +0x08, then loads the shared mode byte 0x8014686B, shifts it left by two and
 *   stores it into byte +0x07 of the record published at 0x80148210. When that
 *   byte reads nonzero it calls the shared status helper func_8015C148 once and
 *   branches on the returned word: on any answer other than 2 it calls the
 *   shared arming helper func_801ABBA8 with the published record and the work
 *   record byte +0x08 as the level and sets bit 0x80 in the published record's
 *   byte 0, while on the answer 2 it calls the shared view routine
 *   func_801C5DFC with that same work byte, clears the shared facing halfword
 *   0x80149330 and sets bit 0x40 in the published record's byte 0. On every path
 *   it then reloads the scratchpad cell, clears the work record's word +0x14
 *   and stores 0x480 into its halfword +0x3E. The 0x18-byte frame keeps only
 *   $ra at +0x10. Takes no arguments and returns nothing.
 *
 * Naming evidence: this span is the fifth word (index 4) of the eight-word
 *   handler run based at 0x801F6398 in the textbin data tail, which the payload
 *   stores little-endian as 0x801F4790, 0x801F4804, 0x801F4864, 0x801F48DC,
 *   this span, 0x801F4AB4, 0x801F4B40 and 0x801F4F7C. The shared mode byte
 *   0x8014686B is the cell the exact neighbour at 0x801F4B40 clears and the
 *   exact neighbour at 0x801F4864 steps, and that neighbour's own comment
 *   records this span as the handler that scales it by four into the record
 *   published at 0x80148210. Byte +0x08 of the scratch work record is the level
 *   index the exact World04 area188 lift (0x801F4840) passes to the same arming
 *   helper, and this body stores the level 1 there before passing it, which is
 *   where the name's level word comes from. func_801ABBA8 is the shared arming
 *   helper the World00 area000 and World04 area188 lifts name, func_801C5DFC is
 *   the shared view routine the exact World03 area144 lift (0x801F3C78) names,
 *   and the shared halfword 0x80149330 with the work record word +0x14 are the
 *   facing pair the reset step at 0x801F564C clears. The caller is unnamed: a
 *   scan of this payload found no `jal` or `j` whose target is 0x801F49C8 and
 *   no `lui 0x801F` pair materialising the run base 0x801F6398 or its pointer
 *   cell 0x801F63E8, so the payload holds this address in the run's own word
 *   only and the body takes no arguments with the roles left structural.
 *   Whether external code reads that entry, and with which index byte, is
 *   unproven.
 *
 * Shape evidence: the exact neighbour at 0x801F48DC, lifted with a 100.00
 *   match, is this body. One payload cmp of the two 0xEC-byte spans differs in
 *   exactly two words: the level it stores into work byte +0x08 (5 there, 1
 *   here) and its internal `j` word, which shifts by the 0xEC offset delta
 *   between the two spans and so reaches the same relative target. Every other
 *   word, including the equality branch that leaves the arming block as the
 *   fall-through and the view block as the taken arm, is identical, so this
 *   seed copies that neighbour's statement order, operand forms, offset-local
 *   spelling and declaration style and writes its own level from this span's
 *   bytes.
 *
 * Clone declined: the two spans are not byte-identical, so the twin is no clone
 *   source; its two differing words are a carried constant and a self-relative
 *   branch encoding, which is shape evidence rather than a duplicate body. A
 *   scan of the extracted payloads for a masked equal of this 0xEC body was
 *   therefore not spent - the nearest body is in this target and already has a
 *   source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte8Level1PublishModeScaledByte7ThenWhenStatusIs2SetViewClearFacingAndFlag40ElseArmAndFlag80ThenClearWorkWord14AndStore480World01Area04813_801F49C8(
    void) {
  u8* record;

  workRecordCursor1F800044[8] = 1;
  D_80148210[7] = D_8014686B << 2;
  if (D_80148210[7] != 0) {
    if (func_8015C148() != 2) {
      func_801ABBA8(D_80148210, workRecordCursor1F800044[8]);
      D_80148210[0] |= 0x80;
    } else {
      func_801C5DFC(workRecordCursor1F800044[8]);
      D_80149330 = 0;
      D_80148210[0] |= 0x40;
    }
  }
  record = workRecordCursor1F800044;
  *(u32*)(record + 0x14) = 0;
  *(u16*)(record + 0x3E) = 0x480;
}
