#include "bof3/bof3.h"

/* Shared overlay state byte of the main executable, outside this payload. The
 * world04/area171/13 handler at 0x801F3F24, lifted exact, sets it to 1 in front
 * of the same request helper, and this target's generated undefined-syms map
 * binds the address already.
 * @source 0x8014933B @kind data
 */
extern u8 D_8014933B;

/* Front-end entry table of the main executable at 0x80143FC8: 0x74-byte rows
 * whose byte 0 is the occupancy flag and whose byte 5 is the handler selector
 * of the table dispatched at 0x801C7C70. The seeded record family states those
 * two bytes; the +0x34/+0x38 word pair is the row position. This target's
 * generated undefined-syms map binds the address.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];

/* Shared two-value request helper of the main executable, outside this payload
 * and reached from this body only. Its role is unresolved, so the parameters
 * stay structural. The generated undefined-funcs map binds the address.
 * @source 0x8015507C
 */
extern void func_8015507C(s32 arg0, s32 arg1);

/* Front-end record allocator of the main executable: returns the index of a
 * free row of the 0x74-byte table at 0x80143FC8, or 0xFF when no row is free.
 * The generated undefined-funcs map binds the address.
 * @source 0x8019601C
 */
extern u8 func_8019601C(void);

/* Name evidence: the body seeds one row of the 0x74-byte front-end entry table
 * at 0x80143FC8 exactly as the exact seeded-record family does - allocator
 * func_8019601C, the 0xFF test on its masked result, the 0x74 index scale, the
 * single row-base pointer, occupancy byte 0 and handler selector byte 5 - so it
 * is named for that seed, for the request pair 0x4A/0x2F it passes first and for
 * the state byte it raises. This body writes the position pair as constants
 * instead of copying the work object's, so the name says fixed position.
 *
 * Shape evidence: the statement order, the byte-offset row spelling and the
 * declaration style come from the exact sibling
 * seedFrontEndRecordHandler1CFromWorkPositionWorld00Area01013_801F2CDC, whose
 * first four row statements this body repeats with its own values; the store
 * order is this span's own. The discriminating selector 0xB7 appears in no
 * other payload, so no byte-identical twin exists to clone: a word compare of
 * this span against all 13058 generated sibling disassemblies leaves one near
 * sibling, this target's still-unlifted func_801F369C at 0x801F369C, whose
 * single differing word is that same addiu literal (0x9B there).
 *
 * Entry: no jal in this payload names the body. The word 0x801F3724 at
 * 0x801F4F10 is the eighth entry of the ascending code-pointer run that starts
 * at 0x801F4EF4 inside the payload's script/data blob at 0x801F409C, so a
 * dispatcher reached from outside this payload can select it and the blob
 * carries no prototype.
 *
 * @source 0x801F3724
 * @behavior Overlay front-end seed. It raises the shared overlay state byte
 *           0x8014933B to 1, passes the pair 0x4A and 0x2F to the shared
 *           two-value request helper func_8015507C, and then asks the front-end
 *           record allocator func_8019601C for a row index. Unless that index
 *           reads 0xFF, it seeds that 0x74-byte row of the table at 0x80143FC8
 *           with handler selector 0xB7 at byte 5, the fixed position word
 *           0x460000 at 0x34, the occupancy flag 1 at byte 0 and the fixed
 *           position word 0x340000 at 0x38. The four cells do not overlap, so
 *           the stored values are the whole effect and their order is
 *           unobservable; the source keeps the order the original stores them
 *           in. The 0x18-byte frame saves $ra at 0x14 and $s0 at 0x10, and $s0
 *           carries the constant 1 across both calls because that one value
 *           feeds both the state byte and the occupancy flag. It takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSharedStateByteAndRequestPair4a2fThenSeedFrontEndRecordHandlerB7AtFixedPositionWorld02Area10013_801F3724(
    void) {
  u8* record;
  u8 index;

  D_8014933B = 1;
  func_8015507C(0x4A, 0x2F);
  index = func_8019601C();
  if (index != 0xFF) {
    record = &D_80143FC8[(u32)index * 0x74u];
    record[5] = 0xB7;
    *(u32*)(record + 0x34) = 0x460000;
    record[0] = 1;
    *(u32*)(record + 0x38) = 0x340000;
  }
}
