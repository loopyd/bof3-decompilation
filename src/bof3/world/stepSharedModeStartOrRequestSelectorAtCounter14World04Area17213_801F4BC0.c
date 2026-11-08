#include "bof3/bof3.h"

/* Shared front-end mode byte this overlay steps. The dispatch reads it with lb,
 * so it is declared signed here while every arm stores it with a plain byte
 * store; the mode pointer also reaches the shared flag bank as a byte offset. */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared counter byte the finish arms wait on. */
extern u8 D_80146867; /* @source 0x80146867 @kind data */

/* Shared flag byte the finish arms clear. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

void func_801BE1B0(u32 arg0);
void func_8015B580(void* flag_bank, u8 bit_index);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F4BC0
 * @behavior Steps the shared front-end mode byte 0x801448EC through this
 *           overlay's two entry pairs and returns untouched for every other
 *           mode. Mode 0 passes 1 to the shared helper func_801BE1B0 and
 *           advances to mode 1; mode 5 passes 0 to it and advances to mode 6.
 *           Modes 1 and 6 wait until the shared counter byte 0x80146867 reads
 *           0x14 and then raise bit 0x4E of the shared main-RAM flag bank
 *           0x80144F28, which the body reaches as 0x63C bytes past the mode
 *           byte, and request the shared front selector context seed 0xAC
 *           through func_8019FA28 with 0xA0000 and 0x50000 as kind 0x83 from
 *           mode 1 and 0x260000 and 0x360000 as kind 0x82 from mode 6. Both
 *           finish arms share one tail that clears the shared flag byte
 *           0x801448EB and resets the mode byte to 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepSharedModeStartOrRequestSelectorAtCounter14World04Area17213_801F4BC0(
    void) {
  s8* mode;

  mode = &D_801448EC;
  switch (*mode) {
  case 0:
    func_801BE1B0(1);
    *mode = 1;
    return;
  case 1:
    if (D_80146867 != 0x14) {
      return;
    }
    func_8015B580(mode + 0x63C, 0x4E);
    func_8019FA28(0xAC, 0xA0000, 0x50000, 0x83);
    break;
  case 5:
    func_801BE1B0(0);
    *mode = 6;
    return;
  case 6:
    if (D_80146867 != 0x14) {
      return;
    }
    func_8015B580(mode + 0x63C, 0x4E);
    func_8019FA28(0xAC, 0x260000, 0x360000, 0x82);
    break;
  default:
    return;
  }
  D_801448EB = 0;
  *mode = 0;
}
