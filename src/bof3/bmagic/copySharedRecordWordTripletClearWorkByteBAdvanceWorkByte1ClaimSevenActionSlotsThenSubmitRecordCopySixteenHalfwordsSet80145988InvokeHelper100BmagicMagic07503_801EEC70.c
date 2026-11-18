#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. A store through the loaded pointer may alias the cell, so
 * the original re-materialises the address with its own lui/lw pair at each
 * access group: the pose triplet, the work-byte-0xB clear, the work-byte-1
 * read-modify-write, the work-byte-0xA store and once per slot claim. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer cell at 0x8014638C in main-executable RAM,
 * outside this overlay image and bound by the target's generated
 * undefined_syms_auto.txt row. The loaded pointer stays in a saved register
 * across the two shared calls, so the cell is read once into that register and
 * both calls are handed it. Its role is not proven, so the name keeps the
 * address anchor. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Action-slot table in main RAM, 0x78-byte records from 0x801EC330, reached
 * through flat field symbols with the scaled allocator answer added to each:
 * record byte 0x01, byte 0x09, byte 0x0B and word 0x74. The sibling lifts of
 * the bmagic overlays record the same four fields and the same stride. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC33B[]; /* @source 0x801EC33B @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-RAM byte at 0x80145988, outside this overlay image and bound by
 * the target's generated undefined_syms_auto.txt row. Nothing in this payload
 * reads it back, so its role stays unestablished and the name keeps the address
 * anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Action-slot allocator of the shared main executable at 0x801E590C, called
 * once per slot with request class 1 and code 0x25. The zero-extended low byte
 * of the answer is the claimed slot, scaled by the 0x78-byte record stride, the
 * same two-step use the bmagic sibling lifts record for this allocator. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Shared main-executable submitter at 0x80196718. The body hands it the shared
 * battle record and uses no part of an answer, so its role stays
 * address-anchored. */
extern void func_80196718(u8 *record); /* @source 0x80196718 */

/* Shared main-executable effect submitter at 0x8019651C. The body passes the
 * same record with the 0/0/0 offsets and mode 1 and keeps the answer word in
 * $v0 across the following base materialisation, so the declared result type
 * carries that value rather than void. */
extern s32 func_8019651C(u8 *record,
                         s32 arg1,
                         s32 arg2,
                         s32 arg3,
                         s32 arg4); /* @source 0x8019651C */

/* Shared main-executable helper at 0x8015D71C, invoked once with 0x100 and no
 * answer used. Its role is unestablished here, so the name keeps the address
 * anchor. */
extern void func_8015D71C(u16 selector); /* @source 0x8015D71C */

/* @source 0x801EEC70
 * @behavior Entry 0 of the three-entry code pointer run at 0x801EEC04, the
 *           stage the overlay entry dispatcher at 0x801EEC10 selects with work
 *           byte 1, so no jal in this payload names the body. It copies words
 *           0x34, 0x38 and 0x3C of the shared battle record published at
 *           0x8014638C into the same three words of the scratchpad work object
 *           read from 0x1F800044, clearing work byte 0xB between the word 0x3C
 *           load and its store, and increments work byte 1 with a byte-width
 *           read-modify-write, so the next frame runs the following stage. It
 *           then claims seven action slots through the shared allocator
 *           func_801E590C(1, 0x25) in a do-while loop whose exit tests the
 *           incremented ordinal against 7: each iteration stores the pre-step
 *           ordinal into slot byte 0x01, ordinal times five plus one into slot
 *           byte 0x09, zero into slot byte 0x0B and the work-object pointer into
 *           slot word 0x74, with the slot address scaled by the 0x78-byte
 *           record stride from the allocator answer and the work pointer
 *           re-read from the cell on every pass. It submits the shared battle
 *           record through func_80196718, stores the low answer byte of
 *           func_8019651C(record, 0, 0, 0, 1) into work byte 0xA, copies the 16
 *           unsigned halfwords of the table at 0x80036C00 into the table at
 *           0x8003AC00, stores 1 into the shared main-RAM byte 0x80145988 and
 *           invokes func_8015D71C with 0x100. Takes no arguments and returns
 *           nothing; the 0x28-byte frame keeps ra at 0x20, the record pointer at
 *           0x1C and the loop ordinal at 0x18, and holds the fifth
 *           func_8019651C argument at 0x10. The roles of work byte 0xB, of the
 *           slot fields 0x09 and 0x0B, of the two halfword tables, of allocator
 *           code 0x25 and of helper code 0x100 remain unestablished, so those
 *           stay literal or address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copySharedRecordWordTripletClearWorkByteBAdvanceWorkByte1ClaimSevenActionSlotsThenSubmitRecordCopySixteenHalfwordsSet80145988InvokeHelper100BmagicMagic07503_801EEC70(
    void) {
  u8 *record;
  u32 *source;
  u32 *work;
  u32 value;
  u8 *step;
  u8 *published;
  u8 index;
  u8 slot;
  u32 slotOffset;

  record = D_8014638C;
  source = (u32 *)record;
  work = (u32 *)g_battle_work;
  work[13] = source[13];
  work[14] = source[14];
  value = source[15];
  ((u8 *)work)[0xB] = 0;
  step = g_battle_work;
  work[15] = value;
  index = 0;
  step[1]++;

  do {
    slot = (u8)func_801E590C(1u, 0x25u);
    slotOffset = slot << 4;
    slotOffset -= slot;
    slotOffset <<= 3;
    published = g_battle_work;
    D_801EC33B[slotOffset] = index;
    D_801EC339[slotOffset] = index * 5 + 1;
    index++;
    D_801EC331[slotOffset] = 0;
    *(u32 *)&D_801EC3A4[slotOffset] = (u32)published;
  } while (index < 7);

  func_80196718(record);
  g_battle_work[0xA] = (u8)func_8019651C(record, 0, 0, 0, 1);

  index = 0;
  do {
    PSX_PTR(u16, 0x8003AC00u)[index] = PSX_PTR(u16, 0x80036C00u)[index];
    index++;
  } while (index < 0x10);

  D_80145988 = 1;
  func_8015D71C(0x100);
}
