#include "bof3/bof3.h"

/* Shared mode byte of the resident front-end/area controller. This body only
 * proves the unsigned lower bound 7 the original tests with `sltiu`.
 * @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* Shared frame counter. This body only proves the low three bits the original
 * masks with `andi $v0,$v0,0x7`.
 * @source 0x80143E6C @kind unknown */
extern u32 D_80143E6C;

/* Scratchpad byte 0 holds the last claimed front-end record index: every
 * seeder in this family stores it there and later row addresses re-read it.
 * @source 0x1F800000 @kind unknown */
extern u8 D_1F800000;

/* Front-end record table at 0x80143FC8 - 0x74 bytes per record (the original
 * scales the allocator index by 0x74). Only the four fields this body writes
 * are declared, each at its own byte base because the original re-materialises
 * `lui $at,0x8014` for every row store.
 * @source 0x80143FC8 @kind bss */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FCF @kind unknown */
extern u8 D_80143FCF[];
/* @source 0x80143FE4 @kind unknown */
extern u8 D_80143FE4[];

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F2CD4
 * @behavior On every eighth frame, while the shared mode byte D_80146866 reads
 * at least 7, claims one front-end record through the shared allocator
 * func_8019601C, publishes the returned index in scratchpad byte 0x1F800000
 * and, unless that index is 0xFF, seeds the claimed 0x74-byte row at
 * 0x80143FC8: byte +0 takes 1 (the occupancy flag the whole family sets), byte
 * +5 takes the handler selector 0x0B, the word at +0x1C takes 5 and byte +7
 * takes 3. Takes no arguments and returns nothing, so both gates and a full
 * table (index 0xFF) leave every row untouched.
 *
 * Evidence: the payload span 0x801F2CD4-0x801F2DDC is one complete function -
 * the scheduled prologue `lui/lbu` of D_80146866 with `addiu $sp,-0x18` between
 * them and `sw $ra,0x10($sp)` in the `bnez` delay slot, one `jal
 * func_8019601C` whose delay slot is a `nop`, and the single epilogue
 * `lw $ra,0x10($sp); addiu $sp,$sp,0x18; jr $ra; nop`. Both gates branch to
 * that one epilogue: `lbu`/`sltiu $v0,$v0,0x7`/`bnez` for the mode byte and
 * `lw`/`andi $v0,$v0,0x7`/`bnez` for the frame counter. The claim-and-publish
 * block is the family's exact form - `andi $v1,$v0,0xFF`, `addiu $v0,$zero,0xFF`
 * and `beq` with `sll $v0,$v1,3` in its delay slot, then the compiler's 0x74
 * index scale `sll 3 / subu / sll 2 / addu / sll 2` on the allocator result -
 * and each of the four row stores re-materialises its own `lui $at` base with
 * the scaled index added (`sb 1` at D_80143FC8, `sb 0xB` at D_80143FCD,
 * `sw 5` at D_80143FE4, `sb 3` at D_80143FCF), so no single computed row base
 * exists in the original. The second, third and fourth row addresses re-read
 * the scratchpad byte (`lbu $v1,0x0($a0)` with its load-delay `nop`) instead of
 * reusing the allocator result, which is why the source keeps them on
 * `*scratch`.
 *
 * Name evidence: record byte 0x05 is the front-end entry table's handler
 * selector - the shared RecordSlot view in include/bof3/ui/game00_internal.h
 * documents 0x05 as the handler selector for the table at 0x801C7C70, and
 * src/bof3/ui/func_8019A0E4.c dispatches that byte through D_801C7C70 - so this
 * body writes handler selector 0x0B. Byte +0 is the occupancy flag the family
 * sets, the word at +0x1C and byte +7 have no consumer inside this payload and
 * stay described by offset and value. D_80146866 is the shared mode byte the
 * family compares against literal modes (0, 1, 2, 3, 0x23, 0x30, 0x80); this
 * body proves only the `>= 7` bound. The only in-image reference to 0x801F2CD4
 * is the word 0x801F2CD4 at payload offset 0x15CC (0x801F41CC), the fourth word
 * of the five-word code-pointer run 0x801F41C0-0x801F41D0, so the body is
 * reached from outside its payload and has no jal caller.
 *
 * Shape evidence: the claim-and-publish block, the index/scratchpad split and
 * the per-field row-store spelling come from the family's exact siblings
 * seedFrontEndRecordHandler25World01Area07113_801F2CF4 (whose leading block is
 * byte-identical up to the second row store) and
 * seedFrontEndRecordHandler1CFromWorkPositionWorld00Area01013_801F2CDC; this
 * function's own gates, stored literals and extra word store come from the
 * original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordHandler0BEveryEighthFrameWhileModeAtLeast7World00Area00113_801F2CD4(
    void) {
  u8* scratch;
  u8 index;

  if (D_80146866 < 7u) {
    return;
  }

  if ((D_80143E6C & 7u) != 0u) {
    return;
  }

  index = func_8019601C();
  scratch = &D_1F800000;
  *scratch = index;

  if (index != 0xffu) {
    D_80143FC8[(u32)index * 0x74u] = 1u;
    D_80143FCD[(u32)*scratch * 0x74u] = 0x0bu;
    *(s32*)(D_80143FE4 + (u32)*scratch * 0x74u) = 5;
    D_80143FCF[(u32)*scratch * 0x74u] = 3u;
  }
}
