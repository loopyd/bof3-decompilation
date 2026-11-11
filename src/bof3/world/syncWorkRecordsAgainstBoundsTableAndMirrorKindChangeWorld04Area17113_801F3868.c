#include "bof3/bof3.h"

/* Scratchpad work-cursor pointer cell. The original re-loads it before the byte
 * read of every block and again before each store, so the source keeps the
 * named symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Mode byte the tail guard tests against 5; the twin lifts of the World01/Area049
 * overlay (0x801F2DCC) and the World03/Area118 overlay (0x801F4CD0) read the
 * same byte.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Base of the 0x140-byte-stride live work-record array this overlay walks.
 * @source 0x80145E90 @kind table
 */
extern u8 D_80145E90[];

/* Current work-record pointer published to the rest of the overlay, and the
 * live record count byte the loop compares against.
 * @source 0x80146250 @kind data
 */
extern u8* D_80146250;
/* @source 0x80146254 @kind unknown */
extern u8 D_80146254;

/* Shared world flag halfwords. Bit 0x2000 of the first one is the hold this
 * handler raises for an active record and clears at entry and at the tail, and
 * the per-record bits of both words gate activation.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;
/* @source 0x8014625A @kind unknown */
extern u16 D_8014625A;

/* The two five-byte bounds records the payload query 0x801F379C walks start
 * here; the kind byte +4 of each is the kind this handler publishes. Record 0
 * carries kind 0x07 and record 1 kind 0x03.
 * @source 0x801F4188 @kind table
 */
extern u8 D_801F4188[];
/* Kind byte +4 of those same five-byte bounds records. The activate branch reads
 * it through this kind-byte base and the refresh branch through byte +4 of the
 * record base above: naming that one byte once makes the compiler hoist its base
 * into a callee-saved register, which costs two instructions and the register the
 * original shares constant 1 in, while the two spellings keep every table base
 * folded into the indexed load of its own site.
 * @source 0x801F418C @kind table
 */
extern u8 D_801F418C[];

/* Work-sequence selector of the shared main executable; it takes the kind byte
 * the handler published and its result is unused here. */
u32 func_8014D8D4(u8 arg0);
/* Shared work-hold release of the main executable. */
u8 func_8014D978(void);
/* Shared record step and per-index record refresh of the main executable. */
void func_801C2538(void);
void func_801C2710(void);
/* Shared work advance of the main executable. */
void func_801B0DFC(void);

/* Position-bounds query of this payload; it returns the index of the five-byte
 * bounds record holding the given work record's position, or 0xFF. */
u8 findWorkPositionBoundsIndexWorld04Area17113_801F379C(u8 index);

/* @source 0x801F3868
 * @behavior Per-frame work-record sync of this area. Clears bit 0x2000 of the
 *   shared world flags D_80146258, keeps the incoming scratchpad work cursor
 *   0x1F800044 in a register and walks the D_80146254 live records of the
 *   0x140-byte array at D_80145E90. Each iteration publishes that record as
 *   D_80146250 and as the scratch cursor and asks the payload query 0x801F379C
 *   for the five-byte bounds record holding it; a 0xFF answer makes the record
 *   absent, which only costs it its bit in the saved cursor bitmask byte 0x0B
 *   and in D_8014625A. A record whose bit is clear in both masks is activated by
 *   raising both bits, setting shared work mode D_80146250[0x11C] to 2, clearing
 *   countdown byte 9, copying the kind byte of its bounds record into byte 8 and
 *   handing that byte to func_8014D8D4. An already active record runs the shared
 *   step func_801C2538 and, for index 0 only, func_801C2710 while its countdown
 *   reads zero; otherwise it re-derives byte 8 the same way and, when the kind
 *   changed, mirrors the motion words +0x0C, +0x10 and +0x14 and reflects the
 *   countdown as 8 minus itself. Every active path then runs func_801B0DFC,
 *   steps the countdown down by one, releases the shared hold through
 *   func_8014D978 and raises bit 0x2000 of D_80146258. The saved cursor is
 *   restored last, and the trailing guard clears bit 0x2000 again when
 *   D_80143BB0 holds 5. Takes no arguments and returns nothing.
 *
 *   The World03/Area118 twin 0x801F4CD0 of this overlay family keeps the same
 *   statement order around a shared flag-bank probe that XORs bit 2 into the
 *   kind; that probe and its bank 0x80144F28 never appear here, so the kind byte
 *   comes straight off the bounds record, and the two mask bits are raised
 *   before the mode store rather than after it, which is this span's own order.
 *   No jal and no word of this payload names the address, so the caller stays
 *   structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void syncWorkRecordsAgainstBoundsTableAndMirrorKindChangeWorld04Area17113_801F3868(
    void) {
  u8* saved;
  u8* record;
  u8* cursor;
  s32 i;
  u32 index;
  u8 bounds;
  u8 kind;

  D_80146258 &= 0xDFFF;
  saved = D_1F800044;
  i = 0;
  if (D_80146254 != 0) {
    do {
      index = i & 0xFF;
      record = &D_80145E90[index * 0x140];
      D_80146250 = record;
      D_1F800044 = record;
      if ((bounds = findWorkPositionBoundsIndexWorld04Area17113_801F379C(
               (u8)index)) == 0xFF) {
        if (((saved[0xB] >> index) & 1) != 0) {
          saved[0xB] &= ~(1 << index);
          D_8014625A &= ~(1 << index);
        }
        goto next_record;
      }
      if (((saved[0xB] >> index) & 1) == 0) {
        if ((((D_80146258 | D_8014625A) >> index) & 1) != 0) {
          goto next_record;
        }
        saved[0xB] |= 1 << index;
        D_8014625A |= 1 << index;
        D_80146250[0x11C] = 2;
        D_1F800044[9] = 0;
        D_1F800044[8] = D_801F418C[bounds * 5];
        func_8014D8D4(D_1F800044[8]);
        goto next_record;
      }
      if (D_1F800044[9] == 0) {
        func_801C2538();
        if (index == 0) {
          func_801C2710();
        }
      } else {
        kind = D_801F4188[bounds * 5 + 4];
        if (D_1F800044[8] != kind) {
          D_1F800044[8] = kind;
          cursor = D_1F800044;
          *(s32*)(cursor + 0xC) = -*(s32*)(cursor + 0xC);
          *(s32*)(cursor + 0x10) = -*(s32*)(cursor + 0x10);
          *(s32*)(cursor + 0x14) = -*(s32*)(cursor + 0x14);
          cursor[9] = 8 - cursor[9];
        }
      }
      func_801B0DFC();
      D_1F800044[9]--;
      func_8014D978();
      D_80146258 |= 0x2000;
    next_record:
      i++;
    } while ((u32)(i & 0xFF) < D_80146254);
  }
  if (D_80143BB0 == 5) {
    D_80146258 &= 0xDFFF;
  }
  D_1F800044 = saved;
}
