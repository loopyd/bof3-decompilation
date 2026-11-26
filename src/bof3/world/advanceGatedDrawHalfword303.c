#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area15213WorkCursor_1F800044;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Both call sites in this overlay pass the mode 0x5C and the signed work
 * halfword 0x30; the drawer's own prologue narrows two arguments with
 * `sll`/`sra 16` and no call site consumes a result. */
void func_801F3F88(s16 arg0, s16 arg1);

/* @source 0x801F37EC
 * @behavior Byte-3 state handler 3, the last of the four words of this overlay's
 *           handler run based at 0x801F4D84 (0x801F3594, 0x801F3614, 0x801F36EC,
 *           0x801F37EC) and the fifteenth and last word of the in-image
 *           code-pointer run from 0x801F4D58 through 0x801F4D90. Adds 0x0A to the
 *           signed halfword at offset 0x30 of the work record published at the
 *           scratchpad pointer cell 0x1F800044, stores the truncated halfword,
 *           and clears work state byte 0x03 once that stored signed value reaches
 *           0xF0. Unless the shared byte 0x801454F2 is nonzero, the byte at
 *           0x80143BB0 reads 2, or bit 0x100 of the halfword at 0x80146258 is
 *           set, it then writes 1 to work byte 0x03 through a second read of the
 *           pointer cell. Finally it reloads that cell once more and passes mode
 *           0x5C and the signed work halfword 0x30 to the overlay drawer
 *           0x801F3F88, discarding any result. No `jal` names this address; only
 *           the pointer word at 0x801F4D90 does. It takes no arguments, returns
 *           nothing and saves only the return address at 0x10 of its
 *           0x18-byte frame. The 0xB0-byte payload span equals the one at
 *           advanceHalfword30By0aAndSetState3To0AtF0OrTo1UnlessGatedThenDrawHalfword30World03Area12114_801F3940
 *           and the one at
 *           advanceHalfword30By0aAndSetState3To0AtF0OrTo1UnlessGatedThenDrawHalfword30World00Area03313_801F385C
 *           at every byte except the two low bytes of the `jal` word at offset
 *           0x98, which encode each overlay's own drawer address; one payload
 *           `cmp` per sibling put those 0xAE remaining bytes equal, so this body
 *           takes that family's statement order with this target's drawer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceGatedDrawHalfword30(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = area15213WorkCursor_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x0A);
  if (*half >= 0xF0) {
    record[3] = 0;
  }
  if (D_801454F2 == 0 && D_80143BB0 != 2 && (D_80146258 & 0x100u) == 0) {
    area15213WorkCursor_1F800044[3] = 1;
  }
  emitter = area15213WorkCursor_1F800044;
  func_801F3F88(0x5C, FIELD_REF(s16, emitter, 0x30));
}
