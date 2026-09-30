#include "bof3/battle/battle03_internal.h"

/* @behavior Resets the current battle scratch work record: stores 4 in its byte
 * +0x29, clears the six words +0x0C..+0x20, copies the global byte D_801462EC
 * into byte +0x08, clears byte +0x48, publishes the signed distance of the
 * position pair +0x34/+0x38 in halfword +0x3E, clears byte +0x2B and reseeds the
 * scratch state bytes +0x01..+0x04 with 2/0/0/0.
 * @source 0x801DEF0C
 * @status partial
 * @match 92.31
 * @residual 48/52 instructions, 208->204 bytes; first mismatch +0x18 is the byte
 * +0x08 value load: the original keeps `lbu v0,D_801462EC` after the six word
 * clears (load-delay `nop`) and reuses the just-freed v0, while this build hoists
 * the byte load above the clears and therefore allocates it to a0; the entry
 * block register assignment and every later statement match. Clean-C register
 * allocation/scheduling residual (allocation class), not a semantic difference.
 */
void func_801DEF0C(void) {
  Battle03LocalWork* work1;
  Battle03LocalWork* work2;
  Battle03LocalWork* work3;
  Battle03LocalWork* work4;
  u8                 mode;
  s16                distance;

  work1 = D_1F800044;
  work1->unk_29 = 4;
  work1->unk_0c = 0;
  work1->unk_10 = 0;
  work1->unk_14 = 0;
  work1->unk_18 = 0;
  work1->unk_1c = 0;
  work1->unk_20 = 0;
  mode = D_801462EC;
  work2 = D_1F800044;
  work2->unk_08 = mode;
  work3 = D_1F800044;
  work3->unk_48 = 0;
  distance = func_8015477C(D_1F800044->unk_34, D_1F800044->unk_38);
  work4 = D_1F800044;
  work4->unk_3e = distance;
  work4->unk_2b = 0;
  D_1F800044->unk_01 = 2;
  D_1F800044->unk_02 = 0;
  D_1F800044->unk_03 = 0;
  D_1F800044->unk_04 = 0;
}
