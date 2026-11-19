#include "bof3/bof3.h"

typedef void (*Boss03216Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * unlifted handler at 0x800C1A08: it tests the 0x2 bit of the byte at
 * 0x80146328, builds a 0x18-byte frame and then reads the byte records at
 * 0x80145E90 and 0x80145FD0, so its role stays address-anchored. The second is the overlay's battle setup
 * hook, which clears the 0x40 flag of owner record 0, publishes the record into
 * 0x80146884 and 0x1F800044 and requests resource 0. The third is an inert
 * accessor returning a zero result, which the void slot type drops. */
void func_800C1A08(void); /* @source 0x800C1A08 */
void publishOwnerRecordAndRequestResourceZeroBossBoss03216_800C1BEC(
    void); /* @source 0x800C1BEC */
s32 zeroResultBossBoss03216_800C1A00(void); /* @source 0x800C1A00 */

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss03216Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03216Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03216Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C19C8
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * the unlifted handler func_800C1A08 (0x18-byte frame, tests the 0x2 bit of the
 * byte at 0x80146328) into 0x801463A4, the battle setup hook
 * publishOwnerRecordAndRequestResourceZeroBossBoss03216_800C1BEC (clears the
 * 0x40 flag of owner record 0, publishes the owner-0 record into 0x80146884 and
 * the scratchpad cursor 0x1F800044 and requests resource 0) into 0x801463A8,
 * and the zero-result accessor zeroResultBossBoss03216_800C1A00 into
 * 0x801463AC; that accessor's s32 result is dropped by the void slot type. Same
 * trio shape as the exact sibling installers of this archive family, which
 * write the same three slots in the same order and differ only in the published
 * addresses. No in-payload caller: neither this function's address nor the
 * three slot addresses appear as a word of this payload and no jal reaches it,
 * so the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss03216_800C19C8(void) {
  D_801463A4 = func_800C1A08;
  D_801463A8 = publishOwnerRecordAndRequestResourceZeroBossBoss03216_800C1BEC;
  D_801463AC = (Boss03216Handler)zeroResultBossBoss03216_800C1A00;
}
