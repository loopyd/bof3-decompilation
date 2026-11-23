#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-materializes the cell address (lui $vN,0x1F80 / lw $vN,0x44)
 * before each store group, so the cell is read once per access.
 * @source 0x1F800044 @kind data
 */
extern u8 *g_battle_work;

/* Shared battle work pointer cell of the main executable. The original reaches
 * the pointed-to words +0x34 and +0x38 and the pointed-to halfword +0x3E
 * through the loaded pointer instead of at the constant address.
 * @source 0x801EB4E0 @kind data
 */
extern u8 *D_801EB4E0;

/* Scratchpad word the two angular sample helpers take as their argument. This
 * body publishes the scaled work byte into it and re-reads it for the second
 * sample, so it keeps a pointer local rather than two constant accesses.
 * @source 0x1F800000 @kind data
 */
extern s32 D_1F800000;

extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */
extern void func_8014D6B8(u8 value); /* @source 0x8014D6B8 */

/* @source 0x801EF0D0
 * @behavior Handler entry 0 of the two-entry code-pointer run based at
 *           0x801EF7A4 that callStateByte2Tablef7
 *           selects by scaling the work object's unsigned byte 2 by four; no jal
 *           in this payload names this span, so that run alone reaches it. It
 *           decrements the scratchpad work object's unsigned byte 9 and returns
 *           while the truncated result is non-zero. On zero it reads that work
 *           object's unsigned byte 0xB, masks it with 7, scales it by 512,
 *           publishes that angle word into the scratchpad word at 0x1F800000 and
 *           passes the same word to the two angular sample helpers. It offsets
 *           work words 0x34 and 0x38 from the corresponding words of the shared
 *           battle work record at 0x801EB4E0 by each sample times five, scaled by
 *           0x2000 and truncated by twelve bits. It adds 0x180 to that record's
 *           halfword 0x3E and stores the sum into work halfword 0x3E. It then
 *           seeds work byte 0x25 with 0x1D, byte 0x27 with 0xA0, bytes 0x24 and
 *           0x29 with 4 and byte 0x2B with 1, clears work halfword 0x2C and work
 *           bytes 0x26, 0x28, 0x2A and 0x5C to 0x5F, submits work byte 0xB
 *           divided by three to the shared helper func_8014D6B8 and advances work
 *           byte 2. Each store group re-reads the scratchpad cell, so the pointer
 *           is rebuilt before every group. Takes no arguments and returns
 *           nothing; the 0x18-byte frame only keeps $ra at 0x14 and $s0 at 0x10,
 *           $s0 holding the address of the scratchpad angle word.
 *           Twin scan: 26 payloads under out/binaries share this span's first 24
 *           bytes - the prologue and the countdown - and none equals the
 *           0x1D8-byte span, so it was seeded from the original bytes.
 *           Representation: the two halfword accesses into the work object are
 *           written as u16 array-index views of the work pointer. The byte-offset
 *           cast forms, *(u16 *)(work + 0x3E) and *(u16 *)(work + 0x2C), measured
 *           111 of 120 instructions in 480 bytes: the store was then scheduled
 *           before the next pointer load and left an unfilled load delay slot.
 *           The array-index views reproduce the original schedule and matched
 *           every byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementEffectAngularSamples(void) {
  u8 *work;
  s32 *sampleWord;
  s32 sample;

  work = g_battle_work;
  if (--work[9] != 0) {
    return;
  }
  sampleWord = &D_1F800000;
  sample = (g_battle_work[0xB] & 7) << 9;
  *sampleWord = sample;
  sample = func_801782FC(sample);
  *(s32 *)(g_battle_work + 0x34) =
      *(s32 *)(D_801EB4E0 + 0x34) + ((sample * 5 << 13) >> 12);
  sample = func_801783C8(*sampleWord);
  *(s32 *)(g_battle_work + 0x38) =
      *(s32 *)(D_801EB4E0 + 0x38) + ((sample * 5 << 13) >> 12);
  work = g_battle_work;
  ((u16 *)work)[0x1F] = ((u16 *)D_801EB4E0)[0x1F] + 0x180;
  work[0x25] = 0x1D;
  g_battle_work[0x26] = 0;
  g_battle_work[0x27] = 0xA0;
  g_battle_work[0x28] = 0;
  g_battle_work[0x5D] = 0;
  g_battle_work[0x5E] = 0;
  g_battle_work[0x5F] = 0;
  g_battle_work[0x5C] = 0;
  g_battle_work[0x2A] = 0;
  g_battle_work[0x29] = 4;
  g_battle_work[0x24] = 4;
  work = g_battle_work;
  work[0x2B] = 1;
  ((u16 *)work)[0x16] = 0;
  func_8014D6B8((u32)g_battle_work[0xB] / 3);
  g_battle_work[2]++;
}
