#include "bof3/bof3.h"

/* Shared front-end mode byte this overlay steps. The dispatch reads it with
 * `lb`, so it is declared signed here while every arm stores it with a plain
 * byte store. */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared phase byte the mode-1 arm waits on to leave 2. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Shared state-entry byte the release tail clears beside the mode byte. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

void func_80150224(s32 action_id);
void func_8015C058(void);

/* @source 0x801F3180
 * @behavior Steps the shared front-end mode byte 0x801448EC through this
 *           overlay's two-mode area-entry sequence. Mode 0 requests the area
 *           action 1 through func_80150224, latches the shared phase byte
 *           0x80143BB0 to 2 and advances the mode byte to 1. Mode 1 returns
 *           while that phase byte still reads 2; once it differs, the arm
 *           releases the overlay through the shared func_8015C058 helper and
 *           clears the shared state-entry byte 0x801448EB together with the
 *           mode byte. Every other mode byte value returns untouched. Takes no
 *           arguments and returns nothing.
 *
 * Shape: the reviewed sibling func_801F2C38, at the same base address in
 *        world00/area016/13, carries the same switch on the signed byte
 *        0x801448EC with the same two arms: its mode-0 arm requests an action
 *        through func_80150224, writes 2 to 0x80143BB0 and advances the mode
 *        byte, and its mode-1 arm waits for the phase byte to leave 2 and then
 *        runs func_8015C058 and clears 0x801448EB and the mode byte. This span
 *        supplies the statement order, operand forms and declaration style;
 *        this body's own literals are action 1, the direct store of 1 to the
 *        mode byte instead of the sibling's increment, and a release tail that
 *        clears no 0x801448ED byte. The original keeps the mode byte's address
 *        in $s0 for the whole body, because both arms store through it after a
 *        call, so the seed holds it in a local pointer the sibling family uses
 *        for the same reason; the address form was the first gate's only miss.
 *        A whole-payload scan finds neither the address word 0x801F3180 nor a
 *        `jal` encoding that reaches it, so its caller sits outside this
 *        payload and every role stays structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepSharedModeStartAction1ThenReleaseOverlayOnPhaseChangeWorld02Area10513_801F3180(
    void) {
  s8* mode;

  mode = &D_801448EC;
  switch (*mode) {
  case 0:
    func_80150224(1);
    D_80143BB0 = 2;
    *mode = 1;
    break;
  case 1:
    if (D_80143BB0 == 2) {
      break;
    }
    func_8015C058();
    D_801448EB = 0;
    *mode = 0;
    break;
  }
}
