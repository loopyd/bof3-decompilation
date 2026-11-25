#include "bof3/bof3.h"

typedef void (*Boss02316Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the state step at
 * 0x800C19C4, which tests bit 0x2 of the flag byte at 0x80146328 and either
 * writes scenario progress 0xA and calls 0x801DDED8 or calls 0x801DDF00; the
 * second walks owner ids 0, 1 and 2 through the main-program helper at
 * 0x800AC934; the third returns a zero result, which the void slot type drops. */
void func_800C19C4(void);
void stateOwnerRecordHeads(void);
s32  zeroResult13(void);

/* Shared hook slots. 0x801463A4 and 0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss02316Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02316Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02316Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1984
 * @behavior Publishes this overlay's handler trio into the shared hook slots in
 * ascending slot order: the state step at 0x800C19C4 into 0x801463A4, the owner
 * record clear wrapper at 0x800C1A10 into 0x801463A8, and the zero-result
 * accessor at 0x800C19BC into 0x801463AC, whose s32 result the void slot type
 * drops. The body takes no arguments and returns no value.
 *
 * Byte evidence: the 0x38 bytes hold three lui/addiu address materialisations,
 * each followed by a lui/sw pair into the next slot, then jr $ra with a nop
 * delay slot, so the published order is A4, A8, AC. The span is one body with
 * the spans at boss028/16 0x800C1B40 and boss033/16 0x800C1D70: a payload
 * comparison against each found the three addiu low halves at +0x04, +0x14 and
 * +0x24 as the only differing words, and those are exactly the published
 * addresses of this overlay, so those spans supply shape and every literal here
 * comes from this span. No in-payload caller: no jal reaches
 * 0x800C1984 and no other word of the 684-byte payload holds it or the three
 * slot addresses, so the main program reaches this installer through its
 * overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooks19(void) {
  D_801463A4 = func_800C19C4;
  D_801463A8 = stateOwnerRecordHeads;
  D_801463AC = (Boss02316Handler)zeroResult13;
}
