#include "bof3/bof3.h"

/* Overlay handler seeded from two exact siblings that split this span's parts.
 * The guard is the one advanceWorkWord34By8000WhenWorkByte79Is2World01Area04013_801F2C74
 * writes: its first six words are this span's first six (same 0x80146250 cell,
 * same +0x79 byte, same literal 2, same `bne` with a 8-word skip), and it uses
 * the same `Is2` spelling for the equality. The update is the one
 * advanceHalfword0aBy9WhenWorkByte79Not2World00Area01813_801F2C68 writes: its
 * 0x40 bytes carry this same instruction layout word for word, with only the
 * branch polarity (`beq` there, `bne` here) and the increment literal
 * different. A scan of the extracted payloads under out/binaries matches this
 * span's leading words only in world01/area040/13 at 0x74, which diverges at
 * +0x18, so no byte-identical copy of the whole span exists and the body is
 * written from its own bytes. */

/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F307C
 * @behavior Overlay per-frame handler: reads mode byte 0x79 of the work record
 *           published through the global pointer cell 0x80146250 and, only when
 *           that byte is 2, adds 1 to the halfword at offset 0x0A of the record
 *           published through the global pointer cell 0x80148210, reading and
 *           writing that halfword back through the one register that holds the
 *           record. Every other mode returns before the second cell is touched.
 *           The +0x79 byte is the mode selector this overlay family gates on:
 *           the exact siblings advanceWorkWord34By8000WhenWorkByte79Is2World01Area04013_801F2C74
 *           and advanceHalfword0aBy9WhenWorkByte79Not2World00Area01813_801F2C68
 *           read the same cell against the same literal, and the exact sibling
 *           seedWorkPositionSelectingYByWorkByte79ThenSetFacingByte8World03Area12813_801F3030
 *           of this same overlay tests it the same way. The +0x0A halfword is
 *           the counter those halfword-0x0A siblings advance through the same
 *           0x80148210 record. Takes no arguments, returns nothing, calls
 *           nothing and reserves no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword0aBy1WhenWorkByte79Is2World03Area12813_801F307C(void) {
  if (D_80146250[0x79] == 2) {
    D_80148210[5] += 1;
  }
}
