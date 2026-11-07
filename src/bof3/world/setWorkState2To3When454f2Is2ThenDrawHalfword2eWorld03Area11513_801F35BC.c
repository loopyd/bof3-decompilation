#include "bof3/bof3.h"

extern u8 D_801454F2;

/* Draws the work halfword as a signed 16-bit screen coordinate. */
void func_801F3B04(s16 arg0, s16 arg1);

/* @source 0x801F35BC
 * @behavior When the shared mode byte 0x801454F2 reads 2, writes 3 to byte 0x02
 *           of the work record published at the scratchpad pointer slot 0x44.
 *           Then reloads that cell and calls the local 0x801F3B04 drawer with
 *           0x10 and the signed halfword at record offset 0x2E. Takes no
 *           arguments, uses no result and saves only the return address.
 *           The 0x50 bytes are byte-identical to the exact
 *           setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld03Area12114_801F35D0
 *           in emi/world03/area121/14 apart from the low byte of the internal
 *           `jal` word that encodes that overlay's own drawer address, so this
 *           body follows the reviewed sibling's statement order with this
 *           target's drawer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld03Area11513_801F35BC(
    void) {
  u8* record;

  if (D_801454F2 == 2) {
    SPAD_PTR_SLOT(u8, 0x44)[2] = 3;
  }
  record = SPAD_PTR_SLOT(u8, 0x44);
  func_801F3B04(0x10, FIELD_REF(s16, record, 0x2E));
}
