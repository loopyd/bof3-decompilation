#include "bof3/bof3.h"

/* Shared front-end mode byte this overlay steps. Another overlay of the same
 * family (the state controller at 0x801F37E4) drives the same byte and the
 * same halfword countdown, and this body narrows the byte through a signed
 * char before it indexes the table below, which is why the byte is declared
 * signed here while the surrounding payload stores it with plain byte stores. */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared halfword countdown the mode steps below arm and consume. */
extern u16 D_801448EE; /* @source 0x801448EE @kind data */

/* Shared selector byte and step byte the reset step clears. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Shared flag block whose bit 0x28 the cue step sets through 0x8015B580. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Shared pair committed into the fade words below. */
extern u32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern u32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Shared fade/scroll words the two seeding steps publish. */
extern u32 D_80149308; /* @source 0x80149308 @kind data */
extern u32 D_8014930C; /* @source 0x8014930C @kind data */
extern u16 D_8014932E; /* @source 0x8014932E @kind data */
extern u16 D_80149330; /* @source 0x80149330 @kind data */

/* Shared readiness byte the last step gates on. */
extern u8 D_80149332; /* @source 0x80149332 @kind data */

/* Shared frame-selector byte the seeding steps scale by eight. */
extern u8 D_80181BD8; /* @source 0x80181BD8 @kind data */

void func_8015507C(s32 arg0, s32 arg1);
void func_801A4BC0(s16 x, s16 y, u32 size);
void func_8015DF18(u16 arg0);
void func_8015B580(void* arg0, u8 bit_index);
void func_8015C058(void);
void func_801BE1B0(u32 arg0);

/* @source 0x801F3EE8
 * @behavior Steps this overlay's shared front-end mode byte 0x801448EC through
 *           its 24-entry jump table at 0x801F2C04. Modes 1, 3, 5 and 7 queue
 *           the shared cue func_801BE1B0(0), arm the shared halfword countdown
 *           0x801448EE with 10 and advance the mode to 10; mode 10 consumes
 *           that countdown and, at zero, clears the shared selectors
 *           0x801448EB/0x801448EC/0x801448ED and runs the shared release helper
 *           func_8015C058. Mode 20 runs func_8015507C(0x11, 0xB7), arms the
 *           countdown with 0x1E and advances to 0x15. Mode 21 consumes the
 *           countdown and, at zero, publishes the fade pair 0x118000/0xAF0000
 *           at 0x80149308/0x8014930C, clears the shared facing halfword
 *           0x80149330, advances to 0x16 and stores the frame-selector byte
 *           0x80181BD8 scaled by eight at 0x8014932E. Mode 22, while the shared
 *           readiness byte 0x80149332 reads clear, runs func_8015507C(0x11, 0xAD),
 *           the four cursor-rect calls func_801A4BC0(0x11/0x12, 0xAD/0xAE,
 *           0xA1/0xC0), the cue func_8015DF18(0x205) and the flag-bit call
 *           func_8015B580(&D_80144F28, 0x28), arms the countdown with 0x1E and
 *           advances to 0x17. Mode 23 consumes the countdown and, at zero,
 *           commits the shared pair 0x80145EC4/0x80145EC8 into
 *           0x80149308/0x8014930C, clears 0x80149330, advances to 0x18 and
 *           stores the same scaled frame-selector byte at 0x8014932E. Mode 24,
 *           while 0x80149332 reads clear, arms the countdown with 1 and advances
 *           to 10. Every other table slot returns without touching the shared
 *           state.
 *
 * Evidence: the payload span 0x801F3EE8-0x801F40EF is one complete function of
 * 0x258 bytes with a single `jr $ra` epilogue, so it is neither data nor a
 * fragment. The original loads 0x801448EC with `lbu`, subtracts one and narrows
 * the difference with the `sll 24`/`sra 24` pair before the unsigned bound
 * `sltiu 0x18`, then indexes the 24-word table at 0x801F2C04 by that index and
 * `jr`s through it; slots 2, 4, 6, 8, 9 and 11..19 hold the epilogue address,
 * which is the switch default. The countdown steps use the shared family idiom
 * of a halfword pointer whose `lhu`/`addiu -1`/`sh`/`andi 0xFFFF` result is
 * tested against zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSharedModeByteTable2C04World02Area11113_801F3EE8(void) {
  switch (D_801448EC) {
  case 1:
  case 3:
  case 5:
  case 7:
    func_801BE1B0(0);
    D_801448EE = 10;
    D_801448EC = 10;
    break;
  case 10:
    {
      u16* countdown = &D_801448EE;

      *countdown = *countdown - 1;
      if (*countdown != 0) {
        break;
      }
    }
    D_801448EB = 0;
    D_801448EC = 0;
    D_801448ED = 0;
    func_8015C058();
    break;
  case 20:
    func_8015507C(0x11, 0xB7);
    D_801448EE = 0x1E;
    D_801448EC = 0x15;
    break;
  case 21:
    {
      u16* countdown = &D_801448EE;

      *countdown = *countdown - 1;
      if (*countdown != 0) {
        break;
      }
    }
    D_80149308 = 0x118000;
    D_8014930C = 0xAF0000;
    D_80149330 = 0;
    D_801448EC = 0x16;
    D_8014932E = D_80181BD8 << 3;
    break;
  case 22:
    if (D_80149332 != 0) {
      break;
    }
    func_8015507C(0x11, 0xAD);
    func_801A4BC0(0x11, 0xAD, 0xA1);
    func_801A4BC0(0x12, 0xAD, 0xA1);
    func_801A4BC0(0x11, 0xAE, 0xC0);
    func_801A4BC0(0x12, 0xAE, 0xC0);
    func_8015DF18(0x205);
    func_8015B580(&D_80144F28, 0x28);
    D_801448EE = 0x1E;
    D_801448EC = 0x17;
    break;
  case 23:
    {
      u16* countdown = &D_801448EE;

      *countdown = *countdown - 1;
      if (*countdown != 0) {
        break;
      }
    }
    D_80149330 = 0;
    D_801448EC = 0x18;
    D_80149308 = D_80145EC4;
    D_8014930C = D_80145EC8;
    D_8014932E = D_80181BD8 << 3;
    break;
  case 24:
    if (D_80149332 != 0) {
      break;
    }
    D_801448EE = 1;
    D_801448EC = 10;
    break;
  }
}
