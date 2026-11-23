#include "bof3/bof3.h"

/* Shared main-RAM cells below this overlay's load address. The first is the
 * halfword the gate below reads, the second is the base of the shared flag
 * bank the two flag services are handed, and the third is header word offset
 * +0x28 of the resident map block at 0x80104000: a halfword measured in
 * four-byte units to the first record of one map record chain. */
extern u16 D_80143F04; /* @source 0x80143F04 @kind data */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */
extern u16 D_80104028; /* @source 0x80104028 @kind data */

/* Shared main-EXE helpers, all outside this payload and unlifted, so only the
 * call-site prototypes are declared. 0x8015B580 sets one flag of the byte bank
 * it is handed and 0x8015B5A8 clears one; 0x801576FC is the map record
 * consumer that takes a record of the list walked below. */
void func_8015B580(u32 arg0, s32 arg1);
void func_8015B5A8(u32 arg0, s32 arg1);
void func_801576FC(u32* arg0);

/* @source 0x801F3460
 * @behavior Gated on the shared halfword D_80143F04 reading 0x79, it sets flag
 *           0x42 and then clears flag 0x43 of the shared flag bank at
 *           0x80144F28 through the two main-EXE flag services, and then walks
 *           one record chain of the resident map block: the halfword at
 *           0x80104028 is that block's header word offset, counted in
 *           four-byte units, to the first record, every record whose leading
 *           word is nonzero is handed to the main-EXE map record consumer
 *           0x801576FC, and the walk then steps 4 + 4 * (the count in the
 *           upper half of that same leading word) bytes to the next record and
 *           stops on the first zero leading word. Takes no arguments and
 *           returns nothing: no `jal` immediate targets 0x801F3460 and its only
 *           reference is the payload word at offset 0x2738, so its entry roles
 *           stay structural. The 0x18-byte frame keeps $ra at 0x14 and $s0 at
 *           0x10, and $s0 holds the flag-bank address across both flag calls
 *           before it carries the walking record pointer.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Name evidence: the two writes are the flag ids 0x42 and 0x43 handed to the
 * flag services, read here from the `addiu $a1,$zero,0x42` and
 * `addiu $a1,$zero,0x43` delay slots, and the walk is the payload's own loop
 * over the record chain that header offset +0x28 of the resident map block
 * addresses. The consumer role and the "records terminate on a zero word"
 * reading of that chain come from the reviewed runtime finding for the
 * resident map block in docs/specs/runtime/area016-graphics-initialization.md,
 * which names 0x801576FC the consumer of those records and lists the +0x28
 * header field as halfword offsets measured in four-byte units with a zero
 * terminator. The flag services keep the roles their exact in-tree callers
 * carry: setSharedFlag23WhenSharedSelector5IsSetWorld02Area09713_801F2D28 and
 * bootScenarioStateScenarioScena1300_801F7D50, whose own gates measured exact.
 * No evidenced role exists for D_80143F04 in the tree, so the body keeps that
 * read as a bare halfword compare and the name does not claim one.
 *
 * Shape evidence: statement order, operand forms and declaration style follow
 * bootScenarioStateScenarioScena1300_801F7D50, which holds one flag-bank
 * address in $s0 across its two flag calls with the same `(u32)` argument
 * form, and the 0x18-byte frame with $ra at 0x14 and $s0 at 0x10 is that
 * sibling's frame. The map base is materialised as the unsigned constant
 * 0x80104000u, which is the `lui`/`ori` pair at offsets 0x89C and 0x8A8 rather
 * than the `lui`/`addiu` pair a signed constant emits.
 *
 * Clone declined: an exact scan of this 0xA4-byte span over the 398 payloads
 * under out/binaries returned only this payload's own offset 0x860, and a
 * second scan with every `jal` target word masked returned nothing at all, so
 * no twin exists and the body is seeded from these bytes.
 *
 * Representation notes: the losing spellings were measured, not assumed. A word
 * scaled inside the constant sum, (u32*)(0x80104028 * 4 + 0x80104000u), scored
 * 32 of 41 instructions and 148 bytes with its first difference at +0x14,
 * where the original materialises the base before scaling the offset. A byte
 * advance written as (u8*)record + (halfword + 1) * 4 scored 40 of 41 with its
 * first difference at +0x74, the `sll`/`addiu` order the kept spelling
 * reproduces. The kept spelling, the unsigned constant base with pointer
 * arithmetic plus that byte advance, scored 41 of 41 instructions and 164 of
 * 164 bytes with no differing instruction. Two equivalent spellings scored the
 * same: the base written as (u32*)(0x80104000u + D_80104028 * 4), and the
 * word-stepped advance `record += halfword + 1`.
 */
void stateConsumeConditionChain(
    void) {
  u32* record;

  if (D_80143F04 == 0x79u) {
    func_8015B580((u32)&D_80144F28, 0x42);
    func_8015B5A8((u32)&D_80144F28, 0x43);
    record = PSX_PTR(u32, 0x80104000u) + D_80104028;
    while (*record != 0) {
      func_801576FC(record);
      record = (u32*)((u8*)record + 4 + *(u16*)((u8*)record + 2) * 4);
    }
  }
}
