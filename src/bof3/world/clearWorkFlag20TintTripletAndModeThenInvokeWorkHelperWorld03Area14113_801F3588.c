#include "bof3/bof3.h"

/* The release helper at 0x80196718 lives in the simultaneously loaded ETC/GAME
 * overlay, outside this payload; this target's generated
 * undefined_funcs_auto.txt binds that address, so the lift needs no map row.
 * Its own bytes walk the thirty-two 0xC-byte entries of the table at
 * 0x80145BDC and release every entry whose first word equals the argument, so
 * the work record is handed over unmodified and no value comes back. */
void func_80196718(u8* work);

/* Scratchpad work-record pointer cell. Spelled as the named fixed address rather
 * than through the constant-address accessor, because the original re-emits
 * `lui $vN,0x1F80` plus the folded `lw $vN,68($vN)` before every group; the
 * accessor form shares one materialised base address across the three reads.
 * The `D_<ADDRESS>` spelling binds with no map row. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F3588
 * @behavior Overlay step handler reached from data rather than a call: the
 *           payload's function-pointer blob at 0x801F4D38 holds this address at
 *           its third word, 0x801F4D40. It takes no arguments and returns
 *           nothing. It clears bit 0x20 of byte 0 of the scratchpad work record
 *           published through the pointer cell at 0x1F800044, then, re-reading
 *           the cell for that second group, zeroes the record's tint channel
 *           bytes 0x5F, 0x5E and 0x5D and its mode byte 0x5C in that order.
 *           Finally it re-reads the cell once more and hands the record to the
 *           release helper at 0x80196718. Each group re-materialises the cell
 *           because a store through the previously loaded pointer may alias the
 *           cell itself. The 0x18-byte frame holds only $ra at 0x10 across that
 *           call, and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlag20TintTripletAndModeThenInvokeWorkHelperWorld03Area14113_801F3588(
    void) {
  u8* work;
  u8* tint;

  work = D_1F800044;
  work[0] = (u8)(work[0] & 0xDF);
  tint = D_1F800044;
  tint[0x5F] = 0;
  tint[0x5E] = 0;
  tint[0x5D] = 0;
  tint[0x5C] = 0;
  func_80196718(D_1F800044);
}
