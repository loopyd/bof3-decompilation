#include "bof3/bof3.h"

/* @source 0x80146871 @kind data */
extern u8 D_80146871;
/* @source 0x80143E6C @kind data */
extern u32 D_80143E6C;

/* Front-end record table at 0x80143FC8 - 0x74 bytes per record. This body
 * reaches each field through its own byte base because the original
 * re-materialises `lui $at,0x8014` for every row store.
 * @source 0x80143FC8 @kind bss */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FC9 @kind unknown */
extern u8 D_80143FC9[];
/* @source 0x80143FCE @kind unknown */
extern u8 D_80143FCE[];
/* @source 0x80143FD0 @kind unknown */
extern u8 D_80143FD0[];
/* @source 0x80144006 @kind unknown */
extern u8 D_80144006[];

/* Shared cursor pair of the main executable, taken as an array because the body
 * loads both words through one base register.
 * @source 0x80145EC4 @kind data */
extern s32 D_80145EC4[];

extern int rand(void);
/* @source 0x80196070 */
extern void func_80196070(void);
/* Front-end record allocator: answers the index of the next free 0x74-byte row
 * at 0x80143FC8, or 0xFF when the table is full.
 * @source 0x8019601C */
extern u8 func_8019601C(void);
/* Distance helper of the main executable: takes a 16.16 coordinate pair and
 * answers their distance, which this body biases by 0x400.
 * @source 0x8015477C */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801F4D1C
 * @behavior Entry 0 of the three-entry state-1 handler table 0x801F624C that
 *           the overlay step dispatch 0x801F4CD8 indexes with work byte 0x01.
 *           When bit 0 of the shared byte 0x80146871 is set it runs the shared
 *           0x80196070 helper and returns. Otherwise it returns at once while
 *           the low ten bits of the shared tick word 0x80143E6C are non-zero,
 *           and then takes one rand() answer whose low two bits bound the
 *           number of records it claims and whose bits 4-5 it stores as the
 *           facing of every record. Each iteration claims one 0x74-byte
 *           front-end record at 0x80143FC8 through the shared allocator
 *           0x8019601C and returns as soon as that index reads 0xFF, otherwise
 *           storing 1 in row byte 0x00, 0x16 in row byte 0x05, 1 in row byte
 *           0x01, the loop counter in row byte 0x06 and the random facing in
 *           row byte 0x08, and finally the shared distance helper 0x8015477C
 *           applied to the shared cursor pair 0x80145EC4/0x80145EC8, biased by
 *           0x400, in the signed halfword at row offset 0x3E. Takes no
 *           arguments, saves $s0-$s5 and $ra in its 0x38-byte frame and
 *           returns nothing.
 * @status partial
 * @match 75.00
 * @residual The 0x801F4D1C span is byte-identical to the spans carried by
 *           emi/world01/area065/13 0x801F41F4, emi/world04/area152/13
 *           0x801F407C and the area016/13 body 0x801F41BC except one absolute
 *           jump word, and all three carriers stall on the same point, so the
 *           statement order, operand forms and literals here are taken from
 *           that probe and not from a spelling coincidence. What still differs
 *           is allocation only: the built body saves a seventh callee-saved
 *           register because loop-invariant motion hoists the 0xFF allocator
 *           sentinel out of the loop, while the original re-materialises
 *           `addiu $v0,$zero,0xFF` inside the loop, keeps the widened index in
 *           $v1 instead of $v0, and reaches the loop entry test with `blez`
 *           plus a preheader copy of the bound instead of `beqz` on the bound's
 *           home register. About 35 clean-C spellings across 13 probe batches
 *           (bound as a variable, inline and masked in place, signed and
 *           unsigned, byte and word; index as s32/u32/u8 and masked at the
 *           call; sentinel as return, break and explicit mask; cursor as a
 *           hoisted pointer and inline; guard-plus-do/while forms) measured
 *           this same size, so the size and the six saved registers this
 *           candidate keeps for the prologue, latching compare and stores are
 *           the only reachable shape found.
 */
void seedRandomFrontEndRecordsEvery1024TicksUnlessGatedWorld02Area08813_801F4D1C(
    void) {
  u32 value;
  s32 count;
  s32 nibble;
  s32 slot;
  s32 offset;
  s32 i;
  s32* source;

  if ((D_80146871 & 1) != 0) {
    func_80196070();
    return;
  }
  if ((D_80143E6C & 0x3FF) != 0) {
    return;
  }

  value = (u32)rand();
  nibble = (s32)((value & 0x33) >> 4);
  count = (s32)(value & 3);
  source = D_80145EC4;

  for (i = 0; i < count; i++) {
    slot = func_8019601C();
    if (slot == 0xFF) {
      return;
    }
    offset = slot * 0x74;
    D_80143FC8[offset] = 1;
    D_80143FCD[offset] = 0x16;
    D_80143FC9[offset] = 1;
    D_80143FCE[offset] = i;
    D_80143FD0[offset] = nibble;
    *(u16 *)(D_80144006 + offset) =
        (u16)(func_8015477C(source[0], source[1]) + 0x400);
  }
}
