#include "bof3/bof3.h"

/* Work record published at scratchpad pointer cell 0x1F800044. The original
 * loads that cell once and reaches both the byte read and the byte store
 * through the loaded base, so one u8* local holds it here. The cell spelling
 * and the byte count follow the exact siblings that touch the same record:
 * updateWorkByte8FromMaskWorld03Area12114_801F5684 (0x801F5684) reads and masks
 * this byte to 0..7 through SPAD_PTR_SLOT, and
 * setWorkByte8To3When7Else7World02Area09613_801F3580 (0x801F3580) reads and
 * writes it through one local. */

/* @source 0x801F2E18
 * @behavior Advances work byte 8 of the record published at scratchpad pointer
 *           cell 0x1F800044 by one and keeps it inside three bits, so the byte
 *           cycles 0..7. The cell is loaded once and serves both the byte read
 *           and the byte store; the byte is read unsigned (lbu), incremented by
 *           one, masked with 7 and stored back. Leaf with no frame and no calls;
 *           it takes no arguments and leaves the masked byte in $v0 without a
 *           consumer in this overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte8Modulo8World00Area01313_801F2E18(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  work[8] = (work[8] + 1) & 7;
}
