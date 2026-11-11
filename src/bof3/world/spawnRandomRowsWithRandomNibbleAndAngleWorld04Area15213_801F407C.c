#include "bof3/bof3.h"

/* @source 0x80146871 @kind unknown */
extern u8 D_80146871;
/* @source 0x80143E6C @kind data */
extern u32 D_80143E6C;
/* Row table base 0x80143FC8 and the row-byte addresses this function writes.
 * The original re-materialises `lui $at, 0x8014` for every store, so each row
 * byte is addressed through its own address-derived symbol over the one
 * 0x74-byte row offset.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* @source 0x80143FC9 @kind unknown */
extern u8 D_80143FC9[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FCE @kind unknown */
extern u8 D_80143FCE[];
/* @source 0x80143FD0 @kind unknown */
extern u8 D_80143FD0[];
/* Shared coordinate pair: word 0 at 0x80145EC4 and word 1 at 0x80145EC8 are the
 * two coordinates the shared helper 0x8015477C takes in other overlays. Both are
 * re-loaded for every row, so the pair is declared here as one word array.
 * @source 0x80145EC4 @kind unknown
 */
extern s32 D_80145EC4[];

/* @source 0x80196070 */
extern void func_80196070(void);
/* The original masks this answer with 0xFF at the call site, so this target
 * reads a full-word return here; a byte return type compiles the mask away.
 * @source 0x8019601C
 */
extern s32 func_8019601C(void);
/* @source 0x8015477C */
extern s16 func_8015477C(s32 coord_x, s32 coord_y);

/* @source 0x801F407C
 * @behavior Gated spawner for rows of the shared 0x74-byte table based at
 *           0x80143FC8. While bit 0 of the shared byte at 0x80146871 is set it
 *           calls the shared helper 0x80196070 and returns, and while the
 *           shared word at 0x80143E6C reads nonzero under mask 0x3FF it
 *           returns without spawning. Otherwise it takes one rand() answer,
 *           keeps (answer & 0x33) >> 4 as the nibble it stamps into every row
 *           and answer & 3 as the row count, and for each row asks the shared
 *           helper 0x8019601C for a slot, abandons the loop when that masked
 *           (0xFF) answer is 0xFF, scales the slot by the 0x74-byte stride and
 *           stores, in this order, 1 into row byte 0x00, 0x16 into row byte
 *           0x05, 1 into row byte 0x01, the loop index into row byte 0x06 and
 *           the nibble into row byte 0x08, then stores the 0x400-biased answer
 *           of the shared helper 0x8015477C, called with the coordinate pair at
 *           0x80145EC4, into the halfword at row offset 0x3E. It takes no
 *           arguments, saves the return address and six registers in its
 *           0x38-byte frame and returns nothing. Row byte roles, the
 *           0x80143E6C counter role and the 0x80145EC4 pair role in this
 *           overlay are not established, so the accesses stay offset-based.
 * @status partial
 * @match 72.50
 * @residual The compiler hoists the 0xFF sentinel into a seventh callee-saved
 *           register (`li $s6,255`, saved at 0x30) where the original
 *           re-materialises `addiu $v0,$zero,0xFF` inside the loop, and it
 *           allocates the masked row count straight into a callee-saved
 *           register where the original tests it in $v0 and copies it with
 *           `move $s4,$v0`; the entry test is `beqz` where the original has
 *           `blez`. The frame stays 0x38 and every stored word, access width,
 *           call and literal agrees.
 */
void spawnRandomRowsWithRandomNibbleAndAngleWorld04Area15213_801F407C(void) {
  u32 answer;
  s32 nibble;
  s32 count;
  s32 slot;
  u32 offset;
  s32 i;

  if ((D_80146871 & 1) != 0) {
    func_80196070();
    return;
  }

  if ((D_80143E6C & 0x3FF) != 0) {
    return;
  }

  answer = rand();
  nibble = (answer & 0x33) >> 4;
  count = answer & 3;

  for (i = 0; i < count; i++) {
    slot = func_8019601C() & 0xFF;
    if (slot == 0xFF) {
      return;
    }
    offset = (u32)(slot * 0x74);
    D_80143FC8[offset] = 1;
    D_80143FCD[offset] = 0x16;
    D_80143FC9[offset] = 1;
    D_80143FCE[offset] = i;
    D_80143FD0[offset] = nibble;
    *(u16 *)(D_80143FC8 + offset + 0x3E) =
        (u16)(func_8015477C(D_80145EC4[0], D_80145EC4[1]) + 0x400);
  }
}
