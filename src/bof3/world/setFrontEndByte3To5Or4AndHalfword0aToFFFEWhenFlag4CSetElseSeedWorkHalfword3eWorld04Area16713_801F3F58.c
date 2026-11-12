#include "bof3/bof3.h"

/* Front-side byte of the shared 0x80143F00 game-mode tuple in the main
 * executable, outside this overlay's payload. This span branches on the values
 * 2 and 1 and re-reads the byte for the second test, because the shared flag
 * tester called between the tests may change it. The same byte gates the exact
 * sibling 0x801F49F0 on zero. */
extern u8 D_80143F03; /* @source 0x80143F03 @kind data */

/* Shared main-RAM flag bank of the main executable, outside this overlay's
 * payload, used only as the base argument of the shared flag tester and the
 * shared bit clear helper. Copied into a local pointer so each arm of the
 * payload materialises it once in $s0 and passes it to both calls. */
extern u8 D_80144F28[]; /* @source 0x80144F28 @kind data */

/* Front-end record pointer cell of the main executable, outside this overlay's
 * payload. This span writes its byte +3 and its halfword +0x0A, the same
 * published halfword 0x0A that the exact sibling 0x801F42C4 advances. */
extern u8* D_80148210; /* @source 0x80148210 @kind unknown */

/* Scratchpad work record pointer cell. The target's reviewed map does not bind
 * this word, so it stays address-anchored; the sibling overlays that do bind it
 * name the same cell. Each arm reads it once and both stores of a tail go
 * through that one load. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Shared flag tester of the main executable, outside this overlay's payload:
 * returns nonzero when the bit its second argument selects is set in the bank
 * its first argument addresses. */
extern s32 func_8015B5D4(u32 arg0, s32 arg1); /* @source 0x8015B5D4 */

/* Shared bit clear helper of the same bank, called with the bank base and the
 * index of the bit to clear. */
extern void func_8015B5A8(void* flag_bank, u8 bit_index); /* @source 0x8015B5A8 */

/* @source 0x801F3F58
 * @behavior Handles the front-side byte D_80143F03 values 2 and 1 as two
 *           separate tests that each re-read the byte. On either value, when
 *           the shared flag tester func_8015B5D4 reports bit 0x4C of the
 *           main-RAM flag bank D_80144F28 set, it clears that bit through
 *           func_8015B5A8 and stores 5 into byte +3 of the front-end record
 *           published through D_80148210 for the front-side value 2 and 4 for
 *           the value 1, then stores 0xFFFE into that record halfword +0x0A.
 *           When bit 0x4C stays clear it probes bit 0x4B instead: on the
 *           front-side value 2 a clear 0x4B seeds the scratchpad work record
 *           halfword +0x3E through the pointer cell 0x1F800044 with -0x240,
 *           and on the value 1 a set 0x4B seeds the same halfword with 0x9C0;
 *           both then raise flag 0x40 of the work record byte +0x00 from the
 *           value read before the seed. Every other combination, and any other
 *           front-side value, seeds the work record halfword +0x3E with 0x3C0
 *           and touches nothing else. Takes no arguments and returns nothing;
 *           the 0x18-byte frame keeps $ra at +0x14 and $s0 at +0x10, and no
 *           in-image jal reaches this body, so its argument roles stay
 *           structural and none are taken. The role of the seed values and of
 *           flag bit 0x40 stays anchored to the offsets and constants the
 *           original bytes carry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setFrontEndByte3To5Or4AndHalfword0aToFFFEWhenFlag4CSetElseSeedWorkHalfword3eWorld04Area16713_801F3F58(
    void) {
  u8* bank;

  if (D_80143F03 == 2) {
    u8* work;
    u8 flag;
    s16 seed;

    bank = D_80144F28;
    if (func_8015B5D4((u32)bank, 0x4C) != 0) {
      func_8015B5A8(bank, 0x4C);
      D_80148210[3] = 5;
      *(u16*)(D_80148210 + 0xA) = 0xFFFE;
      return;
    }
    if (func_8015B5D4((u32)bank, 0x4B) == 0) {
      work = D_1F800044;
      flag = work[0];
      seed = -0x240;
      FIELD_REF(u16, work, 0x3E) = seed;
      work[0] = flag | 0x40;
      return;
    }
  }
  if (D_80143F03 == 1) {
    u8* work;
    u8 flag;
    s16 seed;

    bank = D_80144F28;
    if (func_8015B5D4((u32)bank, 0x4C) != 0) {
      func_8015B5A8(bank, 0x4C);
      D_80148210[3] = 4;
      *(u16*)(D_80148210 + 0xA) = 0xFFFE;
      return;
    }
    if (func_8015B5D4((u32)bank, 0x4B) != 0) {
      work = D_1F800044;
      flag = work[0];
      seed = 0x9C0;
      FIELD_REF(u16, work, 0x3E) = seed;
      work[0] = flag | 0x40;
      return;
    }
  }
  FIELD_REF(u16, D_1F800044, 0x3E) = 0x3C0;
}
