#include "bof3/bof3.h"

extern u8 D_80143F06;
extern u8 D_80143F03;
extern u8 D_80147ACF;

/* @source 0x801F4D88
 * @behavior Overlay front-side selector, two independent arms, each a
 * conjunction of the two shared front-side bytes. It reads the staged byte
 * 0x80143F06 and the live byte 0x80143F03 and writes one of two constants into
 * the shared byte 0x80147ACF: pending 1 with live 4 stores 0x0C, pending 4 with
 * live 1 stores 0x0E. Every other pair leaves 0x80147ACF alone. Takes no
 * arguments and returns nothing.
 *
 * Evidence: 0x80143F06 is the copy of 0x80143F03 that the front-entry staging
 * routine at 0x8019FAA0 (etc/game/00) publishes at its 0x8019FB48 store, so the
 * pair is the staged and the live front-side reading. D_80143F03's role as
 * byte 5 of the eight-byte box record that contains a point is recorded by
 * func_801A0348; this overlay's payload stores 0x80147ACF with 0x02 and 0x04 at
 * two other sites (world03/area136/13 0x801F33BC, this payload's sibling
 * function at the same two-arm shape) and no shipped payload loads it, so its
 * engine role stays unknown and the name describes the condition and the store.
 * The address 0x801F4D88 is a .word entry at payload 0x3C88, ahead of the byte
 * list 01 00 04 00 05 00 07 00 09 00, in the payload's handler pointer data.
 *
 * Shape evidence: world03/area136/13@0x801F33BC is the same 26-instruction body
 * over the same three cells, differing only in the two stored immediates
 * (0x02/0x04 there); this function keeps the identical branch/load order and
 * writes its own literals.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectCrossedFrontSideStateWorld04Area18813_801F4D88(void) {
  if (D_80143F06 == 1) {
    if (D_80143F03 == 4) {
      D_80147ACF = 0x0C;
    }
  }
  if (D_80143F06 == 4) {
    if (D_80143F03 == 1) {
      D_80147ACF = 0x0E;
    }
  }
}
