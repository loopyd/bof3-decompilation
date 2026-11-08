#include "bof3/bof3.h"

/* Scratchpad work-cursor pointer cell. The original re-loads it before the
 * byte read of every block and again before each store, so the source keeps the
 * named symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Mode byte the tail guard tests against 5; the twin lift of the World01/Area049
 * overlay (0x801F2DCC) reads the same byte.
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

/* Shared main-RAM flag bank the matched record's flag id is tested against
 * through the shared tester func_8015B5D4.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;

/* Six-byte bounds records: bytes +0/+2 bound the first position word, +1/+3 the
 * second, and +4/+5 are the kind id and the flag id this handler publishes.
 * Record 0 holds bounds 0x0D/0x44/0x0E/0x46 with kind 0x01 and flag id 0x85.
 * @source 0x801F5994 @kind table
 */
extern u8 D_801F5994[];

/* The same kind id and flag id as the payload's own field symbols, addressed as
 * one byte symbol per field so each read keeps its own address pair.
 * @source 0x801F5998 @kind table
 */
extern u8 D_801F5998[];
extern u8 D_801F5999[];

/* Shared flag tester of the main executable. */
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* Work-sequence selector of the shared main executable; it takes the work byte
 * the handler published as its kind and its result is unused here. */
u32 func_8014D8D4(u8 arg0);
/* Shared work-hold release of the main executable. */
u8 func_8014D978(void);
/* Shared record step and per-index record refresh of the main executable. */
void func_801C2538(void);
void func_801C2710(void);
/* Shared work advance of the main executable. */
void func_801B0DFC(void);

/* Position-bounds query of this payload; it returns the index of the six-byte
 * bounds record holding the given work record's position, or 0xFF. */
u8 findWorkPositionBoundsIndexWorld03Area11813_801F4C08(u8 index);

/* @source 0x801F4CD0
 * @behavior Per-frame work-record sync of this area. Clears bit 0x2000 of the
 *   shared world flags D_80146258, keeps the incoming scratchpad work cursor
 *   0x1F800044 in a register and walks the D_80146254 live records of the
 *   0x140-byte array at D_80145E90. Each iteration publishes that record as
 *   D_80146250 and as the scratch cursor and asks the payload query
 *   0x801F4C08 for the bounds record holding it; a 0xFF answer makes the record
 *   absent, which only costs it its bit in the saved cursor bitmask byte 0x0B
 *   and in D_8014625A. A record whose bit is clear in both masks is activated by
 *   setting shared work mode D_80146250[0x11C] to 2, clearing countdown byte 9,
 *   raising both bits and copying kind byte D_801F5998 of its six-byte bounds
 *   record into byte 8, with bit 2 of that kind flipped when the flag bank
 *   0x80144F28 reports the paired D_801F5999 id, before the kind is handed to
 *   func_8014D8D4. An already active record runs the shared step func_801C2538
 *   and, for index 0 only, func_801C2710 while its countdown reads zero;
 *   otherwise it refreshes byte 8 the same way and, when the kind changed,
 *   mirrors the motion words +0x0C, +0x10 and +0x14 and reflects the countdown
 *   as 8 minus itself. Every active path then runs func_801B0DFC, steps the
 *   countdown down by one, releases the shared hold through func_8014D978 and
 *   raises bit 0x2000 of D_80146258. The saved cursor is restored last, and the
 *   trailing guard clears bit 0x2000 again when D_80143BB0 holds 5.
 *
 *   This is the World01/Area049 twin 0x801F2DCC of the same overlay family
 *   without that handler's head guard and without its two area-flag probes: the
 *   area banks 0x80144ED0/0x80144EB0 never appear, the flag bank is addressed
 *   directly at 0x80144F28 instead of through an offset of the area bank, and
 *   the only absence test is the 0xFF bounds answer. Those differences, not the
 *   shared statement order, are what the 0x35C body keeps from it. No jal and no
 *   word of this payload names the address, so the caller stays structural.
 *
 *   Shape: the two branches read the same kind byte through two declarations, the
 *   record field 0x801F5998 of the activate path and byte +4 of the six-byte
 *   record 0x801F5994 of the refresh path. Naming it once makes the compiler
 *   hoist the symbol base into a callee-saved register, which grows the frame to
 *   0x28 and adds two instructions; with the two spellings the original 0x20
 *   frame and the folded base of every table read come back.
 * @status exact
 * @match 100.00
 * @residual none
 */
void syncWorkRecordsAgainstBoundsTableAndMirrorActiveKindWorld03Area11813_801F4CD0(
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
      if ((bounds = findWorkPositionBoundsIndexWorld03Area11813_801F4C08(
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
        D_80146250[0x11C] = 2;
        D_1F800044[9] = 0;
        saved[0xB] |= 1 << index;
        D_8014625A |= 1 << index;
        D_1F800044[8] = D_801F5998[bounds * 6];
        if (func_8015B5D4((u32)&D_80144F28, D_801F5999[bounds * 6]) != 0) {
          D_1F800044[8] ^= 4;
        }
        func_8014D8D4(D_1F800044[8]);
        goto next_record;
      }
      if (D_1F800044[9] == 0) {
        func_801C2538();
        if (index == 0) {
          func_801C2710();
        }
      } else {
        kind = D_801F5994[bounds * 6 + 4];
        if (func_8015B5D4((u32)&D_80144F28, D_801F5999[bounds * 6]) != 0) {
          kind ^= 4;
        }
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
