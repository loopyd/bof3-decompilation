#include "bof3/bof3.h"

/* @source 0x8015C088 @kind function */
extern void func_8015C088(void);
/* @source 0x801448EB @kind data */
extern u8 D_801448EB;
/* @source 0x801448EC @kind data */
extern u8 D_801448EC;
/* @source 0x801490C7 @kind data */
extern s8 D_801490C7;
/* @source 0x801490A8 @kind data */
extern u16 D_801490A8;

/* @source 0x801F2FD8
 * @behavior When the signed shared gate byte at 0x801490C7 is zero, calls
 *           the shared front-end startup helper at 0x8015C088, stores 0x38
 *           into state-entry byte 0x801448EB, stores 0xA into progress byte
 *           0x801448EC, and stores 0x49 into status halfword 0x801490A8.
 *           Otherwise stores 0xFFFF into that status halfword without
 *           calling the helper or writing the state-entry and progress bytes.
 *           Takes no arguments and returns nothing.
 *
 * Name evidence: the same-target handler at 0x801F3184 names the helper,
 * state-entry and progress cells. The handler at 0x801F2F84 identifies the
 * shared signed gate byte and unsigned status halfword. The original span
 * supplies the constants and the gate-clear fall-through arm. A pointer at
 * 0x801F3D9C references this callback; the payload contains no direct call
 * to it, so its interface remains structural.
 *
 * Shape evidence: the area170 handler at 0x801F4950 supplies the helper and
 * consecutive byte-store spelling, but writes status before the guard.
 * Here the original writes status at the arm join, after the byte stores.
 * The complete 0x58-byte span has only its own match across extracted
 * payloads, including a little-endian scan masking jump and call words.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStateEntry38ProgressAAndStatus49WhenGateClearElseStatusFFFFWorld03Area12813_801F2FD8(void) {
  if (D_801490C7 == 0) {
    func_8015C088();
    D_801448EB = 0x38;
    D_801448EC = 0xA;
    D_801490A8 = 0x49;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
