#include "bof3/ui/game00_internal.h"

/* @behavior Marks the current palette range's source colors as high-bit colors.
 * @source 0x80196B9C
 * @status partial
 * @match 65.00
 * @residual First mismatch +0x0010: original lbu v1,0x27(a2) then divu $zero,$v1,$v0, current lbu v0,0x27(a2) then divu $zero,$v0,$v1 (value/divisor register pair swapped, so start lands in $a3 instead of the original $t0). The original 3-instruction divisor-zero guard after the divu (bnez $v0; nop; break 7) is emitted by the assembler only under the opt-in MASPSX --expand-div pass, i.e. an object-profile flag owned by config/compiler/object-flags.cmake; that profile edit is requested from the parent and deliberately not applied here. Clean-C body work already converted the residual from entry-index allocation to this register pair: reading the scratchpad work area through a non-volatile u8 pointer view removed the lbu+andi extension pair and two extra nops, and the loop's own pointer copy restored the preheader reload copy (58.33 -> 65.00, first mismatch +0x0C -> +0x10).
 */
void func_80196B9C(void) {
  u8*           work;
  u8            table_index;
  u32           value;
  u32           divisor;
  u32           quotient;
  u32           remainder;
  u32           start;
  u32           stride;
  volatile u16* colors;
  u32           color_index;
  u32           count;
  u8            row;
  u8*           loop_work;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  table_index = work[0x28];
  value = work[0x27];
  divisor = D_801C7AE0[table_index];
  quotient = value / divisor;
  remainder = value % divisor;
  stride = D_801C7AE8[table_index];
  row = stride * remainder;
  start = quotient;
  if ((work[0x24] & 4u) != 0u) {
    start += 0x10u;
  }
  count = D_801C7AD8[table_index] << 4;
  loop_work = work;
  color_index = 1u;
  if (color_index < count) {
    colors = &PSX_PTR(volatile u16, 0x80037800u)[(start & 0xffu) * 256u];
    do {
      colors[row + color_index] |= 0x8000u;
      color_index++;
    } while (color_index < (D_801C7AD8[loop_work[0x28]] << 4));
  }
  paletteStageSerial = 1u;
}
