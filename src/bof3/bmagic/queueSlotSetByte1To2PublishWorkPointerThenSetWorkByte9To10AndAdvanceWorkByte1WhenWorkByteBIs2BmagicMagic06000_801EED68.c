#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The original re-materialises the cell with its own lui / lw pair before
 * each of the four reads this body makes (0x168, 0x1A0, 0x1C4 and 0x1D4), and
 * every reload sits behind a store issued through the pointer loaded before it,
 * so each read stays written at its own site on the named cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Action-slot pool at 0x801EC330, whose 0x78-byte stride these accesses scale
 * by. The original reaches each field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,index*0x78` + `sb|sw %lo(ADDR)($at)` rather than through one shared
 * record base, so every addressed field keeps its own flat symbol: byte +0x01
 * and the word at +0x74 of one entry, the same two fields the exact siblings of
 * this family fill. Splat already binds both addresses through this target's
 * generated undefined_syms_auto.txt. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared action-slot allocator in the main executable: claims a slot for the
 * given kind/mode pair and returns the index of the entry it took, or 0xFF when
 * the pool holds no free entry. This body masks the answer to a byte and never
 * tests it, so the 0xFF arm is not part of this span. */
u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x801EED68
 * @behavior Work-byte-1 handler of this overlay, index 1 of the five-entry code
 *           pointer run at 0x801EEC04 (0x801EECA4, 0x801EED68, 0x801EEE00,
 *           0x801EEE84 and 0x801EEF04, terminated by a zero word) that the
 *           dispatcher func_801EEC34 copies into its 0x30-byte frame and
 *           selects with the scratchpad work object's byte at offset 0x1 scaled
 *           by four. It reads the scratchpad work-object pointer cell
 *           0x1F800044 once, before the 0x18-byte frame exists, and returns as
 *           soon as that object's byte at offset 0xB is not 2: the immediate is
 *           materialised into a register ahead of the branch while the first
 *           argument of the later call fills the branch delay slot. On that
 *           equality it claims an action slot through the shared allocator
 *           func_801E590C(1, 0x12), masks the returned index to a byte and
 *           scales it by the 0x78-byte slot stride: it stores 2 into the entry
 *           byte at pool offset +0x1, re-reads the pointer cell into the entry
 *           word at +0x74, then re-reads the cell twice more to store 0x10 into
 *           work object byte 0x9 and to increment work object byte 0x1. The
 *           allocator answer is never tested against 0xFF. The 0x18-byte frame
 *           keeps only $ra at 0x10 because of the single call, nothing is
 *           returned and no other state is read or written. The roles of work
 *           bytes 1, 9 and 0xB and of the slot fields stay unestablished, so
 *           the byte offsets anchor the name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSlotSetByte1To2PublishWorkPointerThenSetWorkByte9To10AndAdvanceWorkByte1WhenWorkByteBIs2BmagicMagic06000_801EED68(void) {
  u8 *work;
  u8 *slotWork;
  u32 slot;
  u32 offset;

  work = g_battle_work;
  if (work[0xB] == 2) {
    slot = (u8)func_801E590C(1u, 0x12u);
    offset = slot * 0x78;
    slotWork = g_battle_work;
    D_801EC331[offset] = 2;
    *(u32 *)&D_801EC3A4[offset] = (u32)slotWork;
    g_battle_work[9] = 0x10;
    g_battle_work[1]++;
  }
}
