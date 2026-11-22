#include "bof3/bof3.h"

/* Byte table of this payload, read here with the work record's signed halfword
 * at +0x36 as the index. It lies at 0x801F3664, inside the asm span still
 * claimed by func_801F2DE8, and Splat binds the address through this target's
 * generated undefined_syms_auto.txt, so this lift needs no target-map row. Its
 * four bytes are 0x08, 0x09, 0x0A and 0x00. */
extern u8 D_801F3664[];

/* Halfword table of this payload, four bytes after the byte table above and
 * read here with the work record's signed halfword at +0x3A as the index; the
 * original scales that index by two. Bound the same generated way. */
extern u16 D_801F3668[];

/* @source 0x801F2DE8 */
extern void func_801F2DE8(void);

/* @source 0x801F3070 */
extern void func_801F3070(void);

/* @source 0x801F2C48
 * @behavior Overlay step of the work record published at the scratchpad
 *           pointer cell 0x1F800044, which it loads once and then keeps for the
 *           whole body. It seeds the record halfword at +0x34 with the byte
 *           that the signed halfword at +0x36 selects out of the payload byte
 *           table at 0x801F3664, seeds the record halfword at +0x3E with the
 *           halfword that the signed halfword at +0x3A selects out of the
 *           payload halfword table at 0x801F3668, advances the dispatch state
 *           byte at +0x02 by one, and then calls func_801F2DE8 and
 *           func_801F3070 with no arguments, discarding any result. The +0x3E
 *           store lands in the delay slot of the first call and so precedes the
 *           read of that field the callee makes. It takes no arguments, returns
 *           nothing, and reads or writes no state beyond the record and the two
 *           tables; the 0x18-byte frame only keeps $ra at 0x10 across the two
 *           calls, and the pointer-cell load precedes frame creation.
 *
 *           Boundary and caller: the word 0x801F2C48 is entry 0 of this
 *           overlay's handler run at 0x801F3658, whose three words are
 *           0x801F2C48, 0x801F2CAC and 0x801F2D28. The target's exact
 *           dispatcher dispatchStateByte2Table3658World02Area08613_801F2C04
 *           indexes that run with the record state byte at +0x02, so the
 *           overlay loader reaches this body and no jal names it; the body
 *           itself ends at the jr $ra of the 0x64-byte row, which is the entry
 *           that follows. The advanced +0x02 byte therefore selects the next
 *           handler of that run, which is a fact about the store's effect, not
 *           a claim about this body's role in the overlay.
 *
 *           Name evidence: the two fields the body seeds are +0x34 and +0x3E,
 *           the store that advances the run is the +0x02 dispatch byte, and the
 *           two callees are still unlifted asm spans of this target, so they
 *           stay address-anchored in the name and in these declarations.
 *
 *           Clone eligibility declined: a little-endian scan of every shipped
 *           payload under out/binaries for this body's table load, index add,
 *           byte-2 advance and the delay-slot +0x3E store finds exactly one
 *           site, 0x801F2C48 of this payload itself, so no byte-identical
 *           sibling exists anywhere and no clone dry-run was owed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkHalfword34AndHalfword3eFromTablesAdvanceWorkByte2ThenCall1de8And3070World02Area08613_801F2C48(
    void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u16*)(work + 0x34) = D_801F3664[*(s16*)(work + 0x36)];
  *(u16*)(work + 0x3E) = D_801F3668[*(s16*)(work + 0x3A)];
  work[2] = work[2] + 1;
  func_801F2DE8();
  func_801F3070();
}
