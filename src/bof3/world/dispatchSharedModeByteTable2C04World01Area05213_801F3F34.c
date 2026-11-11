#include "bof3/bof3.h"

/* Shared front-end mode byte this overlay steps. The dispatch reads it with lb,
 * so it is declared signed here while every arm stores it with a plain byte
 * store. */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared halfword countdown the mode steps below arm and consume. */
extern u16 D_801448EE; /* @source 0x801448EE @kind data */

/* Shared state byte the release tail clears. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* Shared byte the mode-10 arm arms with 2 and the mode-11 arm tests. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Shared progress byte the seeding arms write and the gating arms test. */
extern u8 D_80146867; /* @source 0x80146867 @kind data */

/* Payload-local step byte the seeding arms clear and the release tail sets to
 * -1; the payload keeps it inside the data run at 0x801F44B8. */
extern s8 D_801F5071; /* @source 0x801F5071 @kind data */

/* Shared cue queue of the main executable. */
/* @source 0x80150224 */
extern void func_80150224(s32 arg0);

/* Shared three-argument helper of the main executable. */
/* @source 0x80155038 */
extern void func_80155038(s16 x, s16 y, u32 size);

/* Shared cue queue of the main executable. */
/* @source 0x801A4BC0 */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* Shared bit-clear helper of the main executable: it takes the flag bank base by
 * address and the flag id as its second argument. */
/* @source 0x8015B5A8 */
extern void func_8015B5A8(void* flag_bank, u8 bit_index);

/* Shared release helper of the main executable. */
/* @source 0x8015C058 */
extern void func_8015C058(void);

/* Overlay helper that reseeds the front-end record handler this overlay claims. */
/* @source 0x801F3D24 */
extern void invokeHelperWorld01Area05213_801F3D24(void);

/* @source 0x801F3F34
 * @behavior Steps this overlay shared front-end mode byte 0x801448EC through
 *           the 48-slot jump table the payload keeps at 0x801F2C04. Mode 0 seeds
 *           the progress byte 0x80146867 with 1, arms the shared halfword
 *           countdown 0x801448EE with 0x10, clears the payload step byte
 *           0x801F5071 and advances to mode 1. Mode 1 consumes that countdown
 *           and, while fewer than 4 remain, clears the progress byte and
 *           advances to mode 2. Mode 2 consumes the countdown and, at zero,
 *           clears bit 0x24 of the shared flag bank at 0x80144F28 through the
 *           shared bit-clear helper func_8015B5A8, sets the payload step byte to
 *           -1, runs the shared release helper func_8015C058 and clears the
 *           shared bytes 0x801448EB and 0x801448EC. Mode 10 queues the shared
 *           cue func_80150224(0x39), stores 2 into the shared byte 0x80143BB0
 *           and advances to 0xB. Mode 11 returns untouched while that byte still
 *           reads 2 and otherwise runs the same shared release helper and the
 *           same two shared byte clears. Modes 20, 21 and 22 repeat the 0, 1, 2
 *           trio with progress 0xA, countdown 0x10 and flag id 0x26, advancing
 *           through 0x15 and 0x16; modes 40, 41 and 42 repeat it with progress
 *           0x14 and flag id 0x25, advancing through 0x29 and 0x2A. Mode 45
 *           clears the progress byte and advances to 0x2E. Mode 46, while the
 *           progress byte reads 0x1E, runs the overlay helper func_801F3D24 and
 *           advances to 0x2F. Mode 47, while the progress byte reads 0x28, runs
 *           four func_80155038 and four func_801A4BC0 calls over the 0x86/0x87 by
 *           0x11/0x12 grid with sizes 0 and 0x10, arms the countdown with 0xA and
 *           advances to 0x29. Every other table slot returns without touching
 *           the shared state.
 *
 * Evidence: the payload span 0x801F3F34-0x801F4267 is one complete function of
 * 0x334 bytes with a single `jr $ra` epilogue and one shared return block, so it
 * is neither data nor a fragment. The original loads 0x801448EC with `lb`, tests
 * it with the unsigned bound `sltiu 0x30` and `jr`s through the 48-word table at
 * 0x801F2C04; slots 3..9, 12..19, 23..39, 43 and 44 hold the return block, which
 * is the switch default. The three countdown arms use the shared family idiom of a
 * halfword pointer whose `lhu`/`addiu -1`/`sh`/`andi 0xFFFF` result is tested,
 * and the three release arms reach the bit-clear helper with the bank base
 * derived from that same countdown pointer plus 0x63A.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSharedModeByteTable2C04World01Area05213_801F3F34(void) {
  switch (D_801448EC) {
  case 0:
    D_80146867 = 1;
    D_801448EE = 0x10;
    D_801448EC = 1;
    D_801F5071 = 0;
    break;
  case 1:
    {
      u16* countdown = &D_801448EE;

      *countdown = *countdown - 1;
      if (*countdown < 4) {
        D_80146867 = 0;
        D_801448EC = 2;
      }
    }
    break;
  case 2:
    {
      u16* countdown = &D_801448EE;

      *countdown = *countdown - 1;
      if (*countdown != 0) {
        break;
      }
      func_8015B5A8((u8*)countdown + 0x63A, 0x24);
      D_801F5071 = -1;
      func_8015C058();
      D_801448EB = 0;
      D_801448EC = 0;
    }
    break;
  case 10:
    func_80150224(0x39);
    D_80143BB0 = 2;
    D_801448EC = 0xB;
    break;
  case 11:
    if (D_80143BB0 == 2) {
      break;
    }
    func_8015C058();
    D_801448EB = 0;
    D_801448EC = 0;
    break;
  case 20:
    D_80146867 = 0xA;
    D_801448EE = 0x10;
    D_801F5071 = 0;
    D_801448EC = 0x15;
    break;
  case 21:
    {
      u16* countdown = &D_801448EE;

      *countdown = *countdown - 1;
      if (*countdown < 4) {
        D_80146867 = 0;
        D_801448EC = 0x16;
      }
    }
    break;
  case 22:
    {
      u16* countdown = &D_801448EE;

      *countdown = *countdown - 1;
      if (*countdown != 0) {
        break;
      }
      func_8015B5A8((u8*)countdown + 0x63A, 0x26);
      D_801F5071 = -1;
      func_8015C058();
      D_801448EB = 0;
      D_801448EC = 0;
    }
    break;
  case 40:
    D_80146867 = 0x14;
    D_801448EE = 0x10;
    D_801F5071 = 0;
    D_801448EC = 0x29;
    break;
  case 41:
    {
      u16* countdown = &D_801448EE;

      *countdown = *countdown - 1;
      if (*countdown < 4) {
        D_80146867 = 0;
        D_801448EC = 0x2A;
      }
    }
    break;
  case 42:
    {
      u16* countdown = &D_801448EE;

      *countdown = *countdown - 1;
      if (*countdown != 0) {
        break;
      }
      func_8015B5A8((u8*)countdown + 0x63A, 0x25);
      D_801F5071 = -1;
      func_8015C058();
      D_801448EB = 0;
      D_801448EC = 0;
    }
    break;
  case 45:
    D_80146867 = 0;
    D_801448EC = 0x2E;
    break;
  case 46:
    if (D_80146867 != 0x1E) {
      break;
    }
    invokeHelperWorld01Area05213_801F3D24();
    D_801448EC = 0x2F;
    break;
  case 47:
    if (D_80146867 != 0x28) {
      break;
    }
    func_80155038(0x86, 0x11, 0);
    func_801A4BC0(0x86, 0x11, 0);
    func_80155038(0x86, 0x12, 0);
    func_801A4BC0(0x86, 0x12, 0);
    func_80155038(0x87, 0x11, 0);
    func_801A4BC0(0x87, 0x11, 0x10);
    func_80155038(0x87, 0x12, 0);
    func_801A4BC0(0x87, 0x12, 0x10);
    D_801448EE = 0xA;
    D_801448EC = 0x29;
    break;
  }
}
