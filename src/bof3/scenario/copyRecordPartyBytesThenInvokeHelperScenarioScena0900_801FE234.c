#include "bof3/bof3.h"

/* 0x801C1ABC — party/character helper; its exact role is unresolved, so it
 * stays address-anchored like the sibling scena09 wrappers that call it. */
void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x80146254 @kind unknown — live count of the local work records. */
extern u8 D_80146254;
/* @source 0x80145F09 @kind unknown — party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90. */
extern u8 D_80145F09[];
/* @source 0x80144908 @kind unknown */
extern u8 D_80144908[];

/* @source 0x801FE234
 * @behavior Snapshots the party byte of the live 0x140-byte-stride local work
 * records and then re-invokes the party helper for the first three snapshot
 * slots. It walks the D_80146254 live records, and for each record copies the
 * party byte at +0x79 into D_80144908 and into an eight-byte stack snapshot,
 * re-reading the count on every iteration. It then walks the first three
 * snapshot bytes and calls func_801C1ABC with each byte that is not 7, and
 * finally calls func_801C1630 with no arguments. It takes no arguments and
 * returns nothing; the 0x28-byte frame holds the eight-byte snapshot at
 * $sp+0x10 plus the saved $ra/$s0-$s2, and the count load precedes the frame
 * creation. No in-image jal or image data word of this payload addresses the
 * range, so the overlay invokes it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the scaled stride belongs in a separate `offset` local here.
 * With the stride inline in the D_80145F09 subscript gcc hoists the whole
 * symbol address into a saved register (`lui/addiu` before the loop, then
 * `addu v0,v0,a1; lbu v0,0(v0)`), which costs one instruction (measured
 * 47/55, 220 bytes). Naming the product keeps the original folded form
 * `lui $at,%hi(D_80145F09); addu $at,$at,offset; lbu %lo($at)` per iteration
 * (measured 54/54, 216 bytes, first mismatch none). The D_80144908 store keeps
 * its bare `i` subscript and already folds.
 */
void copyRecordPartyBytesThenInvokeHelperScenarioScena0900_801FE234(void) {
  u8 slots[8];
  u8 i;
  u8 party;
  u32 offset;

  i = 0;
  if (D_80146254 != 0) {
    do {
      offset = i * 0x140;
      party = D_80145F09[offset];
      D_80144908[i] = party;
      slots[i] = party;
      i++;
    } while (i < D_80146254);
  }
  for (i = 0; i < 3; i++) {
    if (slots[i] != 7) {
      func_801C1ABC(slots[i]);
    }
  }
  func_801C1630();
}
