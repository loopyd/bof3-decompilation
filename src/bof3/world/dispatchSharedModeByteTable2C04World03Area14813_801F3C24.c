#include "bof3/bof3.h"

/* Shared front-end mode byte this overlay steps. The dispatch reads it with lb,
 * so it is declared signed here while every arm stores it with a plain byte
 * store.
 * @source 0x801448EC @kind data
 */
extern s8 D_801448EC;
/* Shared front-end state byte the mode-1 arm clears; the overlay's state-entry
 * tuple writes the same pair.
 * @source 0x801448EB @kind data
 */
extern u8 D_801448EB;
/* Shared front-end record index the two seeding arms publish through the
 * shared allocator, and the key their 0x74-byte record rows are indexed by.
 * @source 0x801448ED @kind data
 */
extern u8 D_801448ED;
/* Shared halfword countdown the mode-12 arm arms and the mode-13 and mode-21
 * arms tick.
 * @source 0x801448EE @kind data
 */
extern u16 D_801448EE;
/* Shared byte the entry arms compare against 0x18, 0x20 and zero.
 * @source 0x80146867 @kind data
 */
extern u8 D_80146867;
/* Shared effect bank byte the release arms zero or arm with 0x1F.
 * @source 0x8014832E @kind data
 */
extern u8 D_8014832E;
/* Shared main-RAM flag bank the entry and mode-20 arms reach through
 * func_8015B580, func_8015B5F4 and func_8015B5D4.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;
/* Shared signed halfword the seeding arms copy into the record word at +0x68.
 * @source 0x801492DA @kind data
 */
extern s16 D_801492DA;

/* Front-end record bank based at 0x80143FC8 with 0x74-byte rows: byte +0 is
 * the claimed flag, +5 the row kind and +9 the row timer. The three word
 * channels at +0x64/+0x68/+0x6C of the same row follow.
 * @source 0x80143FC8 @kind data
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind data */
extern u8 D_80143FCD[];
/* @source 0x80143FD1 @kind data */
extern u8 D_80143FD1[];
/* @source 0x8014402C @kind data */
extern u8 D_8014402C[];
/* @source 0x80144030 @kind data */
extern u8 D_80144030[];
/* @source 0x80144034 @kind data */
extern u8 D_80144034[];

/* Shared services of the main executable, outside this payload. */
void func_8015B580(void* flag_bank, u8 bit_index);
void func_8015B5F4(void* flag_bank, u8 bit_index);
u32 func_8015B5D4(void* flag_bank, u8 bit_index);
void func_8015C058(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801BE1B0(u32 arg0);
void func_801C601C(u32 arg0);
s32 func_8019601C(void);
void func_8015DF18(u32 arg0);
void func_8015822C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* @source 0x801F3C24
 * @behavior Steps the shared front-end mode byte 0x801448EC through this
 *           overlay area-entry sequence, selecting the arm from the 22-slot
 *           jump table the payload keeps at 0x801F2C04. Modes 0, 1, 10, 11, 12,
 *           13, 14, 20 and 21 have bodies and every other slot returns
 *           untouched. Mode 0 runs the shared helper func_801BE1B0 with 0 and
 *           advances to 1. Mode 1 waits for the shared byte 0x80146867 to read
 *           0x18, then clears the shared state byte 0x801448EB and the mode
 *           byte, raises flag 0x46 of the main-RAM flag bank through
 *           func_8015B580 - reached as 0x63D bytes past the state byte it just
 *           cleared, which is one register add and not a second symbol
 *           materialisation - and requests the front selector context 0x95 with
 *           0x1C0000, 0xC0000 and kind 0x80. Mode 10 claims a front-end record
 *           through the shared allocator func_8019601C, stores the returned
 *           index in 0x801448ED and, unless that index is 0xFF, seeds the
 *           record row: byte +0 = 1, +5 = 0x13, the word -0x39A at +0x64, the
 *           signed halfword 0x801492DA at +0x68, the word 0x190 at +0x6C and
 *           +9 = 0x30; it then advances to 11. Mode 11 waits for bit 0 of that
 *           row to clear and runs func_801C601C with 2 before advancing to 12.
 *           Mode 12 waits for 0x80146867 to read 0x20, runs the shared bit
 *           service func_8015B580 on bit 0x5A of the same bank and arms the
 *           halfword countdown 0x801448EE with 0x20 before advancing to 13.
 *           Mode 13
 *           ticks that countdown and, at zero, claims and seeds a second
 *           front-end record with the word -0x2AA at +0x64, 0x801492DA at
 *           +0x68, the word 0x200 at +0x6C, +5 = 0x13 and +9 = 0x20 before
 *           advancing to 14. Mode 14 releases the overlay through func_8015C058
 *           and clears the state pair once 0x80146867 reads zero. Mode 20 runs
 *           that bit service on bit 0x6A once more, queues cue 0x204 through
 *           func_8015DF18, and runs the five-argument helper func_8015822C with
 *           (0, 0x380, -6, -4, -4) when the flag tester func_8015B5D4 reports
 *           that bit set and with (0, 0x180, 0, 0, 0) when it reads clear; it
 *           then arms the countdown with 0xF, clears the effect bank
 *           0x8014832E and advances to 21. Mode 21 ticks that countdown, arming
 *           0x1F in the effect bank at zero, and releases the overlay with the
 *           same state-pair clear as mode 14, which the compiler shares as one
 *           trailing block. Takes no arguments and answers nothing while
 *           keeping a 0x20-byte frame that saves ra at 0x1C and s0 at 0x18 for
 *           the mode-20 bank pointer. The mode-10 arm reaches the published
 *           index through a kept pointer to 0x801448ED whose every later read
 *           reloads it, while the mode-13 arm reads the byte symbol directly,
 *           and both seeding arms read the row index fresh for each store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSharedModeByteTable2C04World03Area14813_801F3C24(void) {
  switch (D_801448EC) {
  case 0:
    func_801BE1B0(0);
    D_801448EC = 1;
    break;
  case 1:
    {
      u8* state;

      if (D_80146867 != 0x18) {
        break;
      }
      state = &D_801448EB;
      *state = 0;
      D_801448EC = 0;
      func_8015B580(state + 0x63D, 0x46);
      func_8019FA28(0x95, 0x1C0000, 0xC0000, 0x80);
    }
    break;
  case 10:
    {
      u8* slot;
      u8 record_index;

      record_index = func_8019601C();
      slot = &D_801448ED;
      *slot = record_index;
      if (record_index == 0xFF) {
        break;
      }
      D_80143FC8[(u32)record_index * 0x74] = 1;
      D_80143FCD[(u32)*slot * 0x74] = 0x13;
      *(s32*)(D_8014402C + (u32)*slot * 0x74) = -0x39A;
      *(s32*)(D_80144030 + (u32)*slot * 0x74) = (s32)(s16)D_801492DA;
      *(s32*)(D_80144034 + (u32)*slot * 0x74) = 0x190;
      D_80143FD1[(u32)*slot * 0x74] = 0x30;
      D_801448EC = 11;
    }
    break;
  case 11:
    if (D_80143FC8[(u32)D_801448ED * 0x74] & 1) {
      break;
    }
    func_801C601C(2);
    D_801448EC = 12;
    break;
  case 12:
    if (D_80146867 != 0x20) {
      break;
    }
    func_8015B580(&D_80144F28, 0x5A);
    D_801448EE = 0x20;
    D_801448EC = 13;
    break;
  case 13:
    {
      u16* counter;
      u8 record_index;

      counter = (u16*)&D_801448EE;
      if (--*counter != 0) {
        break;
      }
      record_index = func_8019601C();
      D_801448ED = record_index;
      if (record_index == 0xFF) {
        break;
      }
      D_80143FC8[(u32)record_index * 0x74] = 1;
      D_80143FCD[(u32)D_801448ED * 0x74] = 0x13;
      *(s32*)(D_8014402C + (u32)D_801448ED * 0x74) = -0x2AA;
      *(s32*)(D_80144030 + (u32)D_801448ED * 0x74) = (s32)(s16)D_801492DA;
      *(s32*)(D_80144034 + (u32)D_801448ED * 0x74) = 0x200;
      D_80143FD1[(u32)D_801448ED * 0x74] = 0x20;
      D_801448EC = 14;
    }
    break;
  case 14:
    if (D_80146867 != 0) {
      break;
    }
    func_8015C058();
    D_801448EB = 0;
    D_801448EC = 0;
    break;
  case 20:
    {
      u8* bank;

      bank = &D_80144F28;
      func_8015B5F4(bank, 0x6A);
      func_8015DF18(0x204);
      if (func_8015B5D4(bank, 0x6A) != 0) {
        func_8015822C(0, 0x380, -6, -4, -4);
      } else {
        func_8015822C(0, 0x180, 0, 0, 0);
      }
      D_801448EE = 0xF;
      D_8014832E = 0;
      D_801448EC = 21;
    }
    break;
  case 21:
    {
      u16* counter;

      counter = (u16*)&D_801448EE;
      if (--*counter != 0) {
        break;
      }
      D_8014832E = 0x1F;
      func_8015C058();
      D_801448EB = 0;
      D_801448EC = 0;
    }
    break;
  }
}
