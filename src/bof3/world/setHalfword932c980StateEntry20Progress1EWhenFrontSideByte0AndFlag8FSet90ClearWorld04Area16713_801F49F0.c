#include "bof3/bof3.h"

/* Front-side byte of the shared 0x80143F00 game-mode tuple in the main
 * executable, outside this overlay's payload. This span reads it with `lbu`
 * and does all of its work only while the byte is zero, the same gate the
 * World04Area169 neighbour setStateEntry2FProgress0WhenFrontSideByteIs1And
 * Flag74ClearWorld04Area16913_801F3A28 uses on another value. */
extern u8 D_80143F03; /* @source 0x80143F03 @kind data */

/* Main-RAM flag bank of the main executable, outside this overlay's payload,
 * used only as the base argument of the shared flag tester. Copied into a
 * local pointer so the payload materialises it once in $s0 and passes it to
 * both tests. */
extern u8 D_80144F28[]; /* @source 0x80144F28 @kind data */

/* Shared 16-bit channel of the main executable, outside this overlay's
 * payload. The world lifts advanceHalfword932cTo1500World03Area12114_801F2CBC
 * and retreatChannel932cByA00World00Area01913_801F2C2C read and move the same
 * halfword and its role stays address-anchored, so this span only records that
 * it stores 0x980 there. */
extern u16 D_8014932C; /* @source 0x8014932C @kind data */

/* State entry byte of the shared front-end tuple in the main executable,
 * outside this overlay's payload. The setStatusFFFFAndStateEntry handlers of
 * the world family request their entry through it. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* Progress byte of the same shared front-end tuple, written beside the entry
 * byte by every handler of that family. */
extern u8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared flag tester of the main executable, outside this overlay's payload:
 * returns nonzero when the bit its second argument selects is set in the bank
 * its first argument addresses. The exact World04Area169 neighbour above
 * declares the same prototype. */
extern s32 func_8015B5D4(u32 arg0, s32 arg1); /* @source 0x8015B5D4 */

/* @source 0x801F49F0
 * @behavior Requires the shared front-side byte D_80143F03 to read zero and
 *           the main-RAM flag bank D_80144F28 to hold bit 0x8F while bit 0x90
 *           stays clear, both read through the shared flag tester
 *           func_8015B5D4. Only on that path it stores 0x980 into the shared
 *           halfword D_8014932C, then requests shared state entry 0x20 through
 *           D_801448EB with progress 0x1E in D_801448EC; a nonzero front-side
 *           byte, a clear 0x8F bit or a set 0x90 bit reaches the shared
 *           epilogue having touched nothing. Takes no arguments and returns
 *           nothing. The front-side load is hoisted above the frame setup and
 *           the bank base is materialised once into $s0, which the 0x18-byte
 *           frame saves beside $ra; every path leaves through the single
 *           epilogue whose `jr $ra` keeps a nop delay slot. The payload names
 *           this body only from the word at 0x801F68B0 inside its script blob
 *           and no in-image `jal` reaches it, so its argument roles stay
 *           structural and none are taken.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setHalfword932c980StateEntry20Progress1EWhenFrontSideByte0AndFlag8FSet90ClearWorld04Area16713_801F49F0(
    void) {
  u8* bank;

  if (D_80143F03 != 0) {
    return;
  }
  bank = D_80144F28;
  if (func_8015B5D4((u32)bank, 0x8F) == 0) {
    return;
  }
  if (func_8015B5D4((u32)bank, 0x90) != 0) {
    return;
  }
  D_8014932C = 0x980;
  D_801448EB = 0x20;
  D_801448EC = 0x1E;
}
