#include "bof3/bof3.h"

extern u8 D_80143BB0;
extern u16 D_80143B90;
extern u8 D_8014623B;
extern u16 D_80145AA8;
extern u16 D_80145AB8;
extern u16 D_801F59E4; /* @source 0x801F59E4 @kind data */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* @source 0x801F2C6C
 * @behavior Byte-1 state handler that the overlay's first byte-1 dispatch
 *           table (0x801F5124, indexed by the dispatcher at 0x801F2C04) lists
 *           at entry 1, between the reset handler at entry 0 (0x801F2C48) and
 *           the shared exit tail at entry 2 (0x801F2D78). Returns untouched
 *           while any of the shared mode cells blocks world stepping: the
 *           front-end phase byte at 0x80143BB0 reads 2 or 1, the halfword at
 *           0x80143B90 reads 4 or 3, the unsigned byte at 0x8014623B reads 2,
 *           or the halfwords 0x80145AA8 and 0x80145AB8 overlap. Otherwise it
 *           reads the overlay halfword at 0x801F59E4, whose bit 15 is the flag
 *           the entry 0 handler clears, whose bits 8 to 14 are the phase count
 *           it advances and whose bits 0 to 7 are the counter. The phase count
 *           advances by one and, once it reaches 0x28 with bit 15 set or 0x1E
 *           with bit 15 clear, resets to zero and increments the counter. It
 *           repacks bit 15, the advanced phase count and the counter into
 *           0x801F59E4, and when that stored counter reads 0x0F or more it
 *           increments the work record's state byte at offset 0x01, so the
 *           next dispatch of the same table takes entry 2. Takes no arguments
 *           and returns nothing; the body is a leaf with no frame and the work
 *           pointer at 0x1F800044 is loaded once, only on that last path.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCounterWithinLimit(void) {
  u16 word;
  u16 packed;
  u8 counter;
  u8 flag;
  u32 phase;
  u8* work;

  if (D_80143BB0 == 2 || D_80143B90 == 4 || D_8014623B == 2 ||
      D_80143BB0 == 1 || D_80143B90 == 3 ||
      (D_80145AA8 & D_80145AB8) != 0) {
    return;
  }
  word = D_801F59E4;
  flag = word >> 15;
  counter = word;
  phase = ((word & 0x7F00) >> 8) + 1;
  if ((flag == 0 && phase >= 0x1E) || (flag != 0 && phase >= 0x28)) {
    phase = 0;
    counter++;
  }
  packed = (flag << 15) | (phase << 8) | counter;
  D_801F59E4 = packed;
  if ((packed & 0xFF) >= 0xF) {
    work = D_1F800044;
    work[1]++;
  }
}
