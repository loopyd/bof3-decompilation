#include "bof3/ui/shop00_internal.h"

/* @source 0x801D6CE4
 * @behavior Copies the seven 32-bit words held at byte offset 0xCA0 of the
 *           main-RAM shop state block (both callers pass the block base
 *           0x801448D8) into the 0x1C-byte record `index` of the panel record
 *           table D_801E6098. The index is the byte D_801E6074; the table is
 *           zero-filled in the shipped payload, so it is only written here.
 * @status partial
 * @match 91.18
 * @residual measured 31/34 insns, 136->136 bytes, first=+0x000c; canonical -O2 places the index*28 `sll v0,v0,0x2` after the first four loads and lands the last word in the dying `$a1` instead of `$a2`; no source-shape probe (interleaved, 4+3 grouped, 7 grouped, byte-offset and 2-D/seven-symbol addressing, declaration vs assignment forms) changed that RTL.
 */
void func_801D6CE4(u8 index, u8* state) {
  u32 offset = index * 28;
  u32 w0 = FIELD_REF(u32, state, 0xCA0);
  u32 w1 = FIELD_REF(u32, state, 0xCA4);
  u32 w2 = FIELD_REF(u32, state, 0xCA8);
  u32 w3 = FIELD_REF(u32, state, 0xCAC);

  FIELD_REF(u32, D_801E6098, offset + 0x00) = w0;
  FIELD_REF(u32, D_801E6098, offset + 0x04) = w1;
  FIELD_REF(u32, D_801E6098, offset + 0x08) = w2;
  FIELD_REF(u32, D_801E6098, offset + 0x0C) = w3;
  {
    u32 w4 = FIELD_REF(u32, state, 0xCB0);
    u32 w5 = FIELD_REF(u32, state, 0xCB4);
    u32 w6 = FIELD_REF(u32, state, 0xCB8);

    FIELD_REF(u32, D_801E6098, offset + 0x10) = w4;
    FIELD_REF(u32, D_801E6098, offset + 0x14) = w5;
    FIELD_REF(u32, D_801E6098, offset + 0x18) = w6;
  }
}
