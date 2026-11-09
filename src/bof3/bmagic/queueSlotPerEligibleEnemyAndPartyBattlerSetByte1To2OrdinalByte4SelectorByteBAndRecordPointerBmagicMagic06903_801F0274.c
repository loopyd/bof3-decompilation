#include "bof3/bof3.h"

/* Shared battle selection bytes at 0x80146374 in main executable RAM, outside
 * this overlay image and already bound by the generated undefined_syms_auto.txt.
 * Byte 0 is the acting battler and byte 0x10 the second selection byte; both are
 * compared against the scanned battler selector, so the pair keeps one base and
 * two displacements. */
extern u8 D_80146374[]; /* @source 0x80146374 @kind data */

/* Local party record arrays at 0x80145E90 plus a byte offset scaled by the
 * 0x140-byte party stride: the status halfword at +0x80, the halfword at +0x9A
 * and the flag word at +0x128. The original reaches each as one flat symbol
 * scaled by the stride, so every field keeps its own base. */
extern u8 D_80145F10[]; /* @source 0x80145F10 @kind data */
extern u8 D_80145F2A[]; /* @source 0x80145F2A @kind data */
extern u8 D_80145FB8[]; /* @source 0x80145FB8 @kind data */

/* Enemy record arrays at 0x801EB630 plus a byte offset scaled by the
 * 0x118-byte enemy stride: the status halfword at +0x82, the halfword at +0xAA
 * and the flag word at +0x104. */
extern u8 D_801EB6B2[]; /* @source 0x801EB6B2 @kind data */
extern u8 D_801EB6DA[]; /* @source 0x801EB6DA @kind data */
extern u8 D_801EB734[]; /* @source 0x801EB734 @kind data */

/* Shared battle record pointer cell at 0x801EB4E0 in main executable RAM. The
 * word it holds is published into the claimed slot at +0x74 and the record byte
 * 0x0B is the counter this body raises once per claimed slot; the cell is read
 * again for that increment, so the byte view below is a second cell read. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Action-slot pool at 0x801EC330, whose 0x78-byte stride these accesses scale by.
 * The original reaches each field as `lui $at,%hi(ADDR)` + `addu $at,$at,index`
 * + `sb|sw %lo(ADDR)($at)` rather than through one shared record base, so every
 * addressed field keeps its own flat symbol: bytes +0x01, +0x04 and +0x0B and
 * the pointer word at +0x74 of one entry. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC334[]; /* @source 0x801EC334 @kind data */
extern u8 D_801EC33B[]; /* @source 0x801EC33B @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-executable helpers, both outside this payload. The battler
 * availability test reports a nonzero value for an inactive battler or one whose
 * 0x4000 flag is set, and the slot allocator at 0x801E590C returns the index of
 * the entry it claimed or 0xFF when the pool holds no free entry. */
u32 func_801DB524(u32 arg0); /* @source 0x801DB524 */
u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x801F0274
 * @behavior Claims one action slot for every battler that passes both scans of
 *           this body. The first scan walks the eight enemy battlers, selector
 *           3 to 10, with a byte offset stepped by the 0x118-byte enemy stride;
 *           the second walks the three local party members, selector 0 to 2,
 *           with a byte offset stepped by the 0x140-byte party stride. A battler
 *           is skipped when the shared availability test reports it inactive or
 *           flagged 0x4000, when it equals the acting battler byte at 0x80146374
 *           or the selection byte 0x10 past it, and - in the enemy scan - when
 *           its halfword at +0xAA is not below two, its halfword at +0x82 has no
 *           0x20 bit and its flag word at +0x104 has no 0x4000 bit; the party
 *           scan keeps the same halfword and 0x20 tests at +0x9A and +0x80 and
 *           accepts a flag word at +0x128 carrying either 0x4000 or bit 0. For
 *           every accepted battler the body claims a pool entry through
 *           func_801E590C(1, 0x39), scales the returned index by the 0x78-byte
 *           slot stride and stamps four fields: byte +0x01 becomes 2, byte
 *           +0x04 receives the running count of stamped entries, byte +0x0B
 *           receives the battler selector and the pointer word at +0x74 receives
 *           the shared battle record pointer read from 0x801EB4E0, after which
 *           it increments that record byte 0x0B and the running count. Takes no
 *           arguments, returns nothing, and the 0x28-byte frame keeps $ra at
 *           0x24 and $s0..$s4 at 0x10..0x20 across the calls. The roles of slot
 *           bytes +0x04 and +0x0B, of the scanned halfwords and of the record
 *           byte 0x0B remain unestablished, so the name describes the measured
 *           effect and the unknown fields stay offset-anchored.
 * @status partial
 * @match 81.99
 * @residual 132/161 instructions at the original 644-byte image. The first
 *           difference is +0x0040: the enemy rejection branch targets a latch
 *           four bytes later. In both passes the slot-offset product uses $v1 where
 *           the original keeps it in $v0, so the constant 2, the four indexed
 *           address adds and the store registers pair the opposite way. The
 *           record cell's second read is scheduled after the pointer-word store
 *           instead of before it and costs one nop, and the party pass folds its
 *           two flag bit tests into one `andi 0x4001` where the original keeps
 *           them apart. The 0x28-byte frame, the {ra,s0..s4} save set, both loop
 *           inductions and every branch direction match.
 */
void queueSlotPerEligibleEnemyAndPartyBattlerSetByte1To2OrdinalByte4SelectorByteBAndRecordPointerBmagicMagic06903_801F0274(
    void) {
  s32 index;
  s32 sel;
  u8  created;
  u8 *record;
  u32 offset;
  u32 slot;
  u32 slotOffset;

  created = 0u;
  index = 0;
  {
    u8 *selection = D_80146374;

    sel = 3;
    do {
      offset = index * 0x118;
      if (((u8)func_801DB524((u8)sel) == 0u) && (selection[0] != sel) &&
          (selection[0x10] != sel) &&
          ((*(u16 *)&D_801EB6DA[offset] < 2u) ||
           ((*(u16 *)&D_801EB6B2[offset] & 0x20u) != 0u) ||
           ((*(u32 *)&D_801EB734[offset] & 0x4000u) != 0u))) {
        slot = (u8)func_801E590C(1u, 0x39u);
        slotOffset = slot * 0x78u;
        record = D_801EB4E0;
        D_801EC331[slotOffset] = 2;
        D_801EC334[slotOffset] = created;
        D_801EC33B[slotOffset] = sel;
        *(u32 *)&D_801EC3A4[slotOffset] = (u32)record;
        created++;
        D_801EB4E0[0xB]++;
      }
      sel++;
      index++;
    } while (index < 8);
  }

  index = 0;
  do {
    offset = index * 0x140;
    if (((u8)func_801DB524((u8)index) == 0u) && (D_80146374[0] != index) &&
        (D_80146374[0x10] != index) &&
        ((*(u16 *)&D_80145F2A[offset] < 2u) ||
         ((*(u16 *)&D_80145F10[offset] & 0x20u) != 0u) ||
         ((*(u32 *)&D_80145FB8[offset] & 0x4000u) != 0u) ||
         ((*(u32 *)&D_80145FB8[offset] & 1u) != 0u))) {
      slot = (u8)func_801E590C(1u, 0x39u);
      slotOffset = slot * 0x78u;
      record = D_801EB4E0;
      D_801EC331[slotOffset] = 2;
      D_801EC334[slotOffset] = created;
      D_801EC33B[slotOffset] = index;
      *(u32 *)&D_801EC3A4[slotOffset] = (u32)record;
      created++;
      D_801EB4E0[0xB]++;
    }
    index++;
  } while (index < 3);
}
