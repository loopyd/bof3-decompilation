#include "bof3/bof3.h"

extern s32 D_801FEA74; /* @source 0x801FEA74 @kind data */
extern s32 D_801FEA78; /* @source 0x801FEA78 @kind data */
extern u8 D_80146888[]; /* @source 0x80146888 @kind data */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u8 D_801FE9A0[]; /* @source 0x801FE9A0 @kind data */

/* The caller stores the raw return and never masks it, so this overlay
 * declares the shared slot allocator word-typed; the identifiers it hands out
 * stay in 0x00..0xFF with 0xFF meaning that no slot is left. */
s32 func_8015CA48(void);
void func_801A65A0(u8* entry);
void func_8014D6DC(s32 id, s32 mode);

/* @source 0x801FD07C
 * @behavior Selects the pair of work records this overlay activates. It asks
 * func_8015CA48 for a free local effect slot identifier, publishes it at
 * 0x801FEA74 - one of the two words that the preceding overlay body at
 * 0x801FC45C fills from the selected record 0x34 and 0x38 words - and returns
 * when that identifier is 0xFF. Otherwise it sets the flag byte of the
 * 0x98-byte record of 0x80146888 that the identifier indexes, publishes a
 * second identifier from func_8015CA48 at 0x801FEA78, and when the second one
 * is 0xFF it clears the first record flag byte and returns. With two
 * identifiers it sets the second record flag byte, publishes the first
 * identifier as the halfword at 0x1F800000 and hands the 16-byte activation
 * entry at 0x801FE9A0 to func_801A65A0, which resolves that halfword into the
 * 0x80146888 table, republishes the resolved record at 0x1F800044 and
 * 0x80146884 and applies the entry fields to it, and then requests resource
 * 0x4C with mode 0 through func_8014D6DC. It repeats both steps for the second
 * identifier with the entry at 0x801FE9B0, which is 0x10 bytes past the first
 * one, and resource 0x36. It takes no arguments and returns nothing; the
 * 0x20-byte frame keeps $ra plus $s0, $s1 and $s2, which hold the reused 0xFF
 * and 1 constants, the first entry base and the scratchpad halfword address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimActivateBothTemplates(void) {
  s32 first;
  s32 second;
  s32 offset;
  u8* entry;
  u16* scratch;

  first = func_8015CA48();
  D_801FEA74 = first;
  if (first == 0xFF) {
    return;
  }
  offset = first * 152;
  D_80146888[offset] = 1;
  second = func_8015CA48();
  D_801FEA78 = second;
  if (second == 0xFF) {
    offset = D_801FEA74 * 152;
    D_80146888[offset] = 0;
    return;
  }
  entry = D_801FE9A0;
  scratch = &D_1F800000;
  D_80146888[second * 152] = 1;
  *scratch = D_801FEA74;
  func_801A65A0(entry);
  func_8014D6DC(0x4C, 0);
  *scratch = D_801FEA78;
  func_801A65A0(entry + 0x10);
  func_8014D6DC(0x36, 0);
}
