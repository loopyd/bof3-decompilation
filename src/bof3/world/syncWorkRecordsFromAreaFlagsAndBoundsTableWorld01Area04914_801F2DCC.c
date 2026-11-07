#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind unknown */
extern u16 D_80143F00; /* @source 0x80143F00 @kind unknown */
extern u16 D_80143F10; /* @source 0x80143F10 @kind unknown */
extern u8 D_80144ED0[]; /* @source 0x80144ED0 @kind unknown */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */
extern u8* D_80146250; /* @source 0x80146250 @kind data */
extern u8 D_80146254; /* @source 0x80146254 @kind unknown */
extern u16 D_80146258; /* @source 0x80146258 @kind unknown */
extern u16 D_8014625A; /* @source 0x8014625A @kind unknown */
extern u8 D_801F5C0C[]; /* @source 0x801F5C0C @kind table */
extern u8 D_801F5C0D[]; /* @source 0x801F5C0D @kind table */

s32 func_8015B5D4(u32 arg0, s32 arg1);
u32 func_8014D8D4(u8 arg0);
u8 func_8014D978(void);
void func_801C2538(void);
void func_801C2710(void);
void func_801B0DFC(void);
void func_801F2C80(void);
u8 findWorkPositionBoundsIndexWorld01Area04914_801F2D00(u8 index);

/* @source 0x801F2DCC
 * @behavior Per-frame work-record sync for this area. When D_80143BB0 holds 5
 *           and the context seeds D_80143F00 and D_80143F10 differ, it only
 *           runs the area reset func_801F2C80 and returns. Otherwise it clears
 *           bit 0x2000 of the shared world flags D_80146258, keeps the incoming
 *           scratchpad work cursor 0x1F800044 in a register and walks the
 *           D_80146254 live records of the 0x140-byte array at D_80145E90.
 *           Each iteration publishes that record as D_80146250 and as the
 *           scratch cursor, and treats the record as absent when area flag 0x25
 *           of the bank D_80144ED0, area flag 0x3A of the bank 0x20 bytes below
 *           it, or a 0xFF bounds index from 0x801F2D00 says so: an absent
 *           record only loses its bit in the saved cursor bitmask byte 0x0B and
 *           in D_8014625A. Otherwise a record whose bit is clear in both masks
 *           is activated by setting shared work mode D_80146250[0x11C] to 2,
 *           clearing countdown byte 9, raising both bits and copying kind byte
 *           D_801F5C0C of its six-byte bounds record into byte 8, with bit 2 of
 *           that kind flipped when the flag bank 0x80144F28 holds the paired
 *           D_801F5C0D byte, before the kind is handed to func_8014D8D4. An
 *           already active record runs the shared step func_801C2538 and, for
 *           index 0 only, func_801C2710 while its countdown reads zero,
 *           otherwise it refreshes byte 8 the same way and, when the kind
 *           changed, mirrors the motion words +0x0C, +0x10 and +0x14 and
 *           reflects the countdown as 8 minus itself. Every active path then
 *           runs func_801B0DFC, steps the countdown down by one, releases the
 *           shared hold through func_8014D978 and raises bit 0x2000 of
 *           D_80146258. The saved cursor is restored last, and the trailing
 *           guard clears bit 0x2000 again when D_80143BB0 holds 5. Takes no
 *           arguments and returns nothing.
 * @status partial
 * @match 87.60
 * @residual The frame matches the original 0x28 bytes. The bounds table base
 *           is hoisted into s4 rather than folded into each indexed load, and
 *           the flag bank occupies s3 instead of s4. Both mask sites load a
 *           literal 1 rather than sharing the original loop-invariant s3.
 *           The build is one instruction shorter; branch targets and delay
 *           slots differ. Calls and memory operations retain their order.
 */
void syncWorkRecordsFromAreaFlagsAndBoundsTableWorld01Area04914_801F2DCC(void) {
  u8* saved;
  u8* bank;
  u8* record;
  u8* cursor;
  s32 i;
  u32 index;
  u8 bounds;
  u8 kind;

  if (D_80143BB0 == 5 && D_80143F00 != D_80143F10) {
    func_801F2C80();
    return;
  }
  D_80146258 &= 0xDFFF;
  saved = D_1F800044;
  bank = D_80144ED0;
  i = 0;
  if (D_80146254 != 0) {
    do {
    index = i & 0xFF;
    record = &D_80145E90[index * 0x140];
    D_80146250 = record;
    D_1F800044 = record;
    if (func_8015B5D4((u32)bank, 0x25) != 0 ||
        func_8015B5D4((u32)(bank - 0x20), 0x3A) != 0 ||
        (bounds = findWorkPositionBoundsIndexWorld01Area04914_801F2D00(
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
      D_1F800044[8] = D_801F5C0C[bounds * 6];
      if (func_8015B5D4((u32)(bank + 0x58), D_801F5C0D[bounds * 6]) != 0) {
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
      kind = D_801F5C0C[bounds * 6];
      if (func_8015B5D4((u32)(bank + 0x58), D_801F5C0D[bounds * 6]) != 0) {
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
