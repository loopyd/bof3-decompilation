#include "bof3/bof3.h"

/* Shared front-end mode byte this overlay steps. The dispatch reads it with
 * `lb`, so it is declared signed here while both arms store it with a plain
 * byte store; the body also reaches the shared main-RAM flag bank as 0x63C
 * bytes past the mode byte. */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared counter byte the finish arm waits on. */
extern u8 D_80146867; /* @source 0x80146867 @kind data */

void func_801BE1B0(u32 arg0);
void func_8015B580(void* flag_bank, u8 bit_index);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F4164
 * @behavior Steps the shared front-end mode byte 0x801448EC through this
 *           overlay's two-value start sequence and leaves every other value
 *           untouched. Mode 0 runs the shared GAME.EMI start helper
 *           func_801BE1B0 with 0 and moves the mode byte to 1. Mode 1 returns
 *           while the shared counter byte 0x80146867 still differs from 0x24;
 *           once it reads 0x24 the arm raises flag 0x55 of the shared main-RAM
 *           flag bank 0x80144F28, which the body reaches as 0x63C bytes past
 *           the mode byte, requests the shared front selector context seed
 *           0xAA at 0x60000 and 0x3F0000 with kind 0x82 through func_8019FA28
 *           and moves the mode byte to 0x1E. Both arms converge on one byte
 *           store through the mode-byte pointer, which the original keeps in
 *           its saved register across the calls, so the advance value is
 *           carried in a local and stored once after the switch. No jal
 *           immediate and no other payload word names 0x801F4164, so its
 *           caller sits outside this payload and the parameter roles stay
 *           structural: it takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepSharedModeStartThenSetFlag55AndRequestSelectorAtCounter24World04Area19713_801F4164(
    void) {
  s8* mode;
  u8 next;

  mode = &D_801448EC;
  switch (*mode) {
  case 0:
    func_801BE1B0(0);
    next = 1;
    break;
  case 1:
    if (D_80146867 != 0x24) {
      return;
    }
    func_8015B580(mode + 0x63C, 0x55);
    func_8019FA28(0xAA, 0x60000, 0x3F0000, 0x82);
    next = 0x1E;
    break;
  default:
    return;
  }
  *mode = next;
}
