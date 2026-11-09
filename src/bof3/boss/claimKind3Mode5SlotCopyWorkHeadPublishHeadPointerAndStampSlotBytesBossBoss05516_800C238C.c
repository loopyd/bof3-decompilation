#include "bof3/bof3.h"

/* Slot allocator of the main executable at 0x801E590C. It takes the two
 * identity bytes a queued entry records at +0x06 and +0x05 and returns the
 * index of the entry it claimed in its low byte, or 0xFF when the pool holds no
 * free entry. This body never tests that failure code, so the declaration keeps
 * the raw argument order and the full returned word. */
u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Shared 0x74-byte work head at 0x801EB630 that this body copies into the
 * claimed entry. Only its base and its end (base + 0x70) are used, and the
 * address of the base is published in the entry word at +0x74 as well, so the
 * destination keeps its own materialisation of the same symbol. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */

/* Action-slot pool at 0x801EC330, whose 0x78-byte stride these accesses scale
 * by. The original reaches the copied entry through one base computed from
 * D_801EC330, but reaches each field it stamps afterwards as `lui $at,%hi(ADDR)`
 * + `addu $at,$at,index*0x78` + `sb|sw %lo(ADDR)($at)`, so every stamped field
 * keeps its own flat symbol. */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC335[]; /* @source 0x801EC335 @kind data */
extern u8 D_801EC336[]; /* @source 0x801EC336 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC359[]; /* @source 0x801EC359 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* The 0x74-byte body a slot entry starts with: 29 words, copied by the original
 * as seven 16-byte chunks followed by one trailing word. */
typedef struct {
  u32 word[29];
} Boss05516WorkHead;

/* @source 0x800C238C
 * @behavior Overlay slot-store helper, reached from
 *           queueSlotThenRaisePendingFlagBossBoss05516_800C2340 with both of its
 *           arguments masked to bytes. It claims an action-slot entry through
 *           the main-executable allocator func_801E590C(3, 5) and uses the low
 *           byte of the returned word as the slot index without testing the
 *           0xFF no-free-entry code. It then scales that index by the 0x78-byte
 *           D_801EC330 stride, copies the 0x74-byte work head at D_801EB630
 *           into the entry as seven 16-byte chunks plus a trailing word,
 *           publishes the D_801EB630 base address in the entry word at +0x74,
 *           and stamps the entry bytes +0x01 = arg0, +0x02 = 0, +0x05 = 5,
 *           +0x06 = 3, +0x09 = 0 and +0x29 = arg1. Bytes +0x05 and +0x06 repeat
 *           the two allocator arguments in the order every other caller of
 *           func_801E590C in this tree records them (the battle-side
 *           copySelectedWorkToQueuedSlot claims with (0, 0xF) and stamps 0xF at
 *           +0x05, 0 at +0x06), so they are the claimed entry's mode and kind
 *           bytes. The roles of the copied head, of entry byte +0x01 and of the
 *           two arguments themselves remain unestablished, so the name stays
 *           offset-anchored. Takes two word arguments, returns nothing, and
 *           keeps a 0x20-byte frame holding $s0, $s1 and $ra because of the one
 *           call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimKind3Mode5SlotCopyWorkHeadPublishHeadPointerAndStampSlotBytesBossBoss05516_800C238C(
    s32 arg0, s32 arg1) {
  u8 slot;
  u32 *dst;

  slot = (u8)func_801E590C(3u, 5u);
  dst = (u32 *)(D_801EC330 + slot * 0x78);
  *(Boss05516WorkHead *)dst = *(Boss05516WorkHead *)D_801EB630;
  D_801EC336[slot * 0x78] = 3;
  D_801EC335[slot * 0x78] = 5;
  D_801EC331[slot * 0x78] = arg0;
  D_801EC332[slot * 0x78] = 0;
  D_801EC339[slot * 0x78] = 0;
  *(u32 *)&D_801EC3A4[slot * 0x78] = (u32)D_801EB630;
  D_801EC359[slot * 0x78] = arg1;
}
