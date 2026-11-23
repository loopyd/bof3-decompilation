#include "bof3/bof3.h"

/* Entry table of the shared main-executable data segment: 0x74-byte rows based
 * at 0x80143FC8. Bytes 0 and 1 are the occupancy flag and the entry state byte,
 * byte 5 is the handler selector and bytes 2 to 4 are unestablished. This body
 * materialises the base once and addresses the row it hands back through fixed
 * offsets of that one pointer, so its fields stay off a single row pointer
 * instead of the per-field absolute symbols the other claim bodies use.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];

/* Front-end record allocator of the shared main executable: it answers the
 * index of the next free 0x74-byte row at 0x80143FC8, or 0xFF when every row is
 * occupied. Splat binds its address through this target's generated
 * undefined_funcs_auto.txt.
 * @source 0x8019601C
 */
extern u8 func_8019601C(void);

/* @source 0x801FD610
 * @behavior Claims one free front-end record for handler selector 0x49 and
 *           hands back its row pointer. The body asks the shared main-executable
 *           allocator func_8019601C for a row index, zero-extends that answer to
 *           a byte and forms the row address at 0x80143FC8 plus the index
 *           scaled by the 0x74-byte row stride before it tests anything; unless
 *           the index is 0xFF it then writes row byte 0 with 1, the occupancy
 *           flag, row byte 5 with 0x49, the handler selector the sibling claim
 *           bodies of this same table seed with 0x34 and 0x3A, and row byte 1,
 *           the entry state byte, with the caller's byte argument. The pointer
 *           is returned on both paths, so a full table returns the base plus
 *           0xFF scaled by the stride after leaving every row untouched. Twelve
 *           jal call sites inside func_801FC2D0 of this payload reach it, at
 *           0x801FC46C, 0x801FC4BC, 0x801FC4FC, 0x801FC554, 0x801FC768,
 *           0x801FC7FC, 0x801FCA8C, 0x801FCAA8, 0x801FCAF0, 0x801FCB0C,
 *           0x801FCBC4 and 0x801FCBE0, passing the state codes 0, 1, 0, 2, 9, 9,
 *           3, 4, 5, 8, 6 and 7 as the only argument, six of them as the first
 *           of a pair of claims the same branch makes, and writing the returned
 *           row's position words and work links without ever testing the 0xFF
 *           answer. No word of this payload holds 0x801FD610, so no pointer run
 *           names it. The roles of the state codes and of the fields this body
 *           leaves alone are not established, so the name and this block
 *           describe the effect only.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the row offset is the compiler
 * multiply-by-0x74 chain (sll 3, subu, sll 2, addu, sll 2) built on the
 * zero-extended allocator answer, the base is materialised after that chain and
 * the pointer is the plain addu of the two, which keeps the row pointer in $a0
 * because the fourth argument register is free once the state byte is saved in
 * the jal delay slot. The 0xFF test is emitted last, after the pointer is
 * complete, because the address is needed on both paths as the return value,
 * and the constant 1 for the occupancy flag is materialised in that test's delay
 * slot. The three row stores keep statement order at the constant offsets 0, 5
 * and 1, and the 0x18-byte frame saves $s0 and $ra for the one value the body
 * keeps across the allocator call.
 */
u8 *claimFrontEndRecordHandler49SeedEntryStateAndReturnRowScenarioScena0700_801FD610(
    u8 state) {
  u8 index;
  u8 *row;

  index = func_8019601C();
  row = &D_80143FC8[(u32)index * 0x74u];
  if (index != 0xFFu) {
    row[0] = 1;
    row[5] = 0x49;
    row[1] = state;
  }
  return row;
}
